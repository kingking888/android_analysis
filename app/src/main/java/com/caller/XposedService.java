package com.caller;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;

import com.units.log;

import java.util.HashMap;
import java.util.Map;

public class XposedService extends Service {
    Map<String, IXposedClient> plug_name_2_client = new HashMap();
    Map<serviceBinder, String> service_handle_2_plug_name = new HashMap<>();
    final String lock_list = "lock_list";

    public XposedService() {
    }

    @Override
    public IBinder onBind(Intent intent) {
        // TODO: Return the communication channel to the service.
        log.i("new client bind!");
        return new serviceBinder();
    }

    @Override
    public boolean onUnbind(Intent intent) {
        log.i("client will unbind!");
        synchronized (lock_list) {
            String plug_name = service_handle_2_plug_name.get(intent);
            if (plug_name != null) {
                service_handle_2_plug_name.remove(intent);
                plug_name_2_client.remove(plug_name);
                log.i("client " + plug_name + " unbind!");
            }
        }
        return true;
    }

    class serviceBinder extends IXposedService.Stub {
        @Override
        public boolean regiest_callback(String plug_name, IXposedClient handle) {
            log("regiest_callback:  " + plug_name + "  " + String.valueOf(handle));
            if (plug_name_2_client.containsKey(plug_name)) {
                return false;
            }
            service_handle_2_plug_name.put(this, plug_name);
            plug_name_2_client.put(plug_name, handle);
            return true;
        }

        @Override
        public void log(String log) {

        }

        @Override
        public boolean ctrl_service(String cmd, byte[] data) throws RemoteException {
            for (IXposedClient client : plug_name_2_client.values()) {
                client.on_callback(cmd, data);
            }
            return true;
        }

        @Override
        public boolean ctrl_xposed_plug(String plug_name, String cmd, byte[] data) throws RemoteException {
            if (!plug_name_2_client.containsKey(plug_name)) {
                return false;
            }
            return plug_name_2_client.get(plug_name).on_callback(cmd, data);
        }
    }
}