package com.units;

import android.util.Log;

import de.robv.android.xposed.XposedBridge;

public class log {
    static String tag = "caller info";

//    public static void xposed(String msg, boolean result) {
//        if (result) {
//            XposedBridge.log(tag + "  " + msg + " success!!!");
//        } else {
//            XposedBridge.log(tag + "  " + msg + " error!!!");
//        }
//    }

    public static void xposed(String msg) {
        XposedBridge.log(tag + "  " + msg);
    }

    public static void i(String msg, boolean result) {
        if (result) {
            Log.i(tag, msg + " success!!!");
        } else {
            Log.i(tag, msg + " error!!!");
        }
    }

    public static void i(String msg) {
        Log.i(tag, msg);
    }

    public static void i(Throwable e) {
        Log.i(tag, Log.getStackTraceString(e));
    }
}
