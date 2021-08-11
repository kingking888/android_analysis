package com.plug.xposed.base;

import android.annotation.SuppressLint;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;

import com.caller.IXposedClient;
import com.caller.IXposedService;

import com.units.log;

public class xposed_plug_base {
    private IXposedService iXposedService;
    private xposed_plug_base self;
    private boolean is_init;
    private String plug_name;

    public Context context;

    public xposed_plug_base() {
        self = this;
    }
 
    IXposedClient client = new IXposedClient.Stub() {
        @SuppressLint("SdCardPath")
        @Override
        public boolean on_callback(String cmd, byte[] args) {
            log.i("on_callback!");
            self.on_callback(cmd, args);
            return true;
        }
    };

    ServiceConnection connection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            iXposedService = IXposedService.Stub.asInterface(service);
            try {
                log.i("regiest_callback!" + String.valueOf(client));
                iXposedService.regiest_callback(plug_name, client);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            log.i("plug service disconnected!!");
        }
    };

    boolean bind_service() {
        log.i("bindService com.caller.XposedService");
        Intent intent = new Intent("com.caller.XposedService");
        intent.setComponent(new ComponentName("com.caller", "com.caller.XposedService"));
        return context.bindService(intent, connection, Context.BIND_AUTO_CREATE);
    }

    public boolean on_callback(String cmd, byte[] args) {
        return false;
    }

    public boolean init_plug(String plug_name, Context context) {
        if (is_init) {
            return false;
        }
        this.context = context;
        this.plug_name = plug_name;
        is_init = true;
        bind_service();
        return true;
    }
}
