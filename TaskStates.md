# Current states #

(to be expanded/explained later)

  * ready to report (boolean)
  * got server ack (boolean)
  * state (integer)
    * new
    * files downloading
    * files downloaded
    * compute error
    * files uploading
    * files uploaded
    * aborted
  * active\_task (sub-object; presence or not of it can be considered an extra boolean state)
    * task\_state (integer)
      * uninitialized
      * executing
      * suspended
      * aborted
      * (others)
    * scheduler state (integer)
      * uninitialized
      * preempted
      * scheduling
    * suspended via GUI (boolean)
    * project suspended via GUI (boolean; actually in PROJECT class but shown in 

&lt;result&gt;

 in the GUI RPCs)