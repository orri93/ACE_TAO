
// $Id$

#if !defined (TAO_AV_BENCH_CLIENT_H)
#define TAO_AV_BENCH_CLIENT_H

#include "ace/Reactor.h"
#include "ace/Event_Handler.h"
#include "ace/ARGV.h"
#include "tao/TAO.h"
#include "orbsvcs/CosNamingC.h"
#include "ace/SOCK_Dgram.h"
#include "orbsvcs/AV/AVStreams_i.h"
#include "ace/SOCK_Acceptor.h"
#include "ace/Synch.h"

#define TCP_PORT 5000

class Client_StreamEndPoint
  : public virtual TAO_Client_StreamEndPoint
{
  // = TITLE
  //    Defines a client StreamEndpoint class
  // = DESCRIPTION
  //    Makes use of a connected dgram to receive data from the server
  //    side stream endpoint.
public:
  Client_StreamEndPoint (void);

  virtual int handle_open (void);
  // called when streamendpoint is instantiated

  virtual int handle_close (void);
  // called when streamendpoint is being destructed

  virtual CORBA::Boolean handle_preconnect (AVStreams::flowSpec &the_spec);
  // called before connecting

  virtual CORBA::Boolean handle_postconnect (AVStreams::flowSpec &the_spec);
  // called after connecting
  
  virtual int handle_start (const AVStreams::flowSpec &the_spec,  
                             CORBA::Environment &env) ;

  virtual int handle_stop (const AVStreams::flowSpec &the_spec,
                            CORBA::Environment &env) ;
  
  virtual int handle_destroy (const AVStreams::flowSpec &the_spec,  
                               CORBA::Environment &env);
  
};

class ttcp_Client_StreamEndPoint;

class ttcp_Acceptor 
  :public virtual ACE_Acceptor <ttcp_Client_StreamEndPoint,ACE_SOCK_ACCEPTOR>
{
public:
  ttcp_Acceptor (ttcp_Client_StreamEndPoint *endpoint);
  
  virtual int make_svc_handler (ttcp_Client_StreamEndPoint *&sh);
private:
  ttcp_Client_StreamEndPoint *endpoint_;
};

class ttcp_Client_StreamEndPoint
  :public Client_StreamEndPoint,
   public virtual ACE_Svc_Handler <ACE_SOCK_STREAM, ACE_NULL_SYNCH>
{
public:
  ttcp_Client_StreamEndPoint (void);
  // constructor
  virtual CORBA::Boolean handle_preconnect (AVStreams::flowSpec &the_spec);
  // called before connecting

  virtual int open (void *);
  // called when server connects to us.
private:
  ttcp_Acceptor acceptor_;
  // The Acceptor.
};

class Client : public ACE_Task<ACE_SYNCH>
// one of these per client thread
{
public:
  Client (int argc, char **argv, ACE_Barrier *barrier);

  virtual int svc (void);

private:
  
  int bind_to_server (void);

  int establish_stream (void);


  TAO_ORB_Manager orb_manager_;

  TAO_AV_Endpoint_Reactive_Strategy_A<Client_StreamEndPoint, TAO_VDev,
    AV_Null_MediaCtrl> reactive_strategy_;

  AVStreams::MMDevice_var server_mmdevice_;

  TAO_MMDevice client_mmdevice_;

  TAO_StreamCtrl streamctrl_;
  // Video stream controller 

  int argc_;

  char **argv_;
  
  ACE_Barrier *barrier_;

};

#endif /* TAO_AV_BENCH_CLIENT_H */
