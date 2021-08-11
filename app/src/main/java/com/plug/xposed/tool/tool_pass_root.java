package com.plug.xposed.tool;

import android.app.Application;
import android.content.Context;
import android.util.Log;

import com.plug.base.plug_config;
import com.plug.xposed.base.sub_plug_base;
import com.units.log;

import java.io.File;

import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XC_MethodReplacement;
import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.XposedHelpers;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

public class tool_pass_root extends sub_plug_base {
    public tool_pass_root(plug_config config, XC_LoadPackage.LoadPackageParam xposed_param) {
        super(config, xposed_param);
        name = "tool_pass_root";
    }

    @Override
    public void on_loadclass(String class_name, Class<?> cls, ClassLoader class_loader, XC_MethodHook.MethodHookParam param) {

    }

    @Override
    public void on_360_reinforce(Context context, ClassLoader class_loader) {

    }

    @Override
    public void on_application_class_loader(Application app, ClassLoader class_loader) {
        XposedHelpers.findAndHookMethod(File.class, "exists", new XC_MethodReplacement() {

            @Override
            protected Object replaceHookedMethod(MethodHookParam param) throws Throwable {
                String file_path = ((File) param.thisObject).getAbsolutePath().toLowerCase();
                log.i("on file exists! " + file_path);
                if (file_path.contains("superuser.apk") ||
                        file_path.contains("/su") ||
                        file_path.contains("xposed") ||
                        file_path.contains("magisk")
                ) {
                    return false;
                }

                return XposedHelpers.callMethod(param.thisObject, "exists");
            }
        });

        XposedHelpers.findAndHookMethod(Runtime.class, "exec", String[].class, String[].class, File.class, new XC_MethodReplacement() {

            @Override
            protected Object replaceHookedMethod(MethodHookParam param) throws Throwable {
                String[] cmds = (String[]) param.args[0];
                String[] envp = (String[]) param.args[1];
                StringBuilder sb = new StringBuilder();
                for (String item : cmds) {
                    sb.append(item);
                    sb.append(" ");
                }
                log.i("on Runtime exec! " + sb.toString());

                if (sb.toString().contains("which") &&
                        sb.toString().contains("su")
                ) {
                    return XposedHelpers.callMethod(param.thisObject, "exec", String.class, "/system/xbin/which xxxxxxxxxxxxxxxxxxxxxx");
                }

                return XposedHelpers.callMethod(param.thisObject, "exec",
                        String[].class, String[].class, File.class,
                        param.args[0], param.args[1], param.args[2]
                );
            }
        });
    }
}
