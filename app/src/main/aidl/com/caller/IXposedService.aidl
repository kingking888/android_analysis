// IXposedService.aidl
package com.caller;

// Declare any non-default types here with import statements
import com.caller.IXposedClient;
interface IXposedService {
   boolean regiest_callback(String plug_name,IXposedClient handle);
   boolean ctrl_service(String cmd,in byte[]data);
   boolean ctrl_xposed_plug(String plug_name,String cmd,in byte[]data);
   void log(String log);
}
