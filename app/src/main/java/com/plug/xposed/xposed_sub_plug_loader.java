package com.plug.xposed;

import android.app.Application;
import android.content.Context;

import com.plug.base.plug_config;
import com.plug.xposed.app_sub_plug.sub_plug_txsd;
import com.plug.xposed.base.sub_plug_base;
import com.plug.xposed.base.xposed_plug_base;
import com.plug.xposed.base.xposed_plug_common;
import com.plug.xposed.tool.tool_inject_jar;
import com.plug.xposed.tool.tool_okhttp_analyse;
import com.plug.xposed.tool.tool_pass_root;
import com.plug.xposed.tool.tool_sslunpinning;
import com.units.log;

import java.util.ArrayList;
import java.util.List;

import de.robv.android.xposed.IXposedHookLoadPackage;
import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.XposedHelpers;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

public class xposed_sub_plug_loader extends xposed_plug_base implements IXposedHookLoadPackage {
    List<sub_plug_base> tools = new ArrayList<>();

    @Override
    public void handleLoadPackage(XC_LoadPackage.LoadPackageParam lpparam) {

        plug_config config = plug_config.load_plug_config();
        if (config == null) {
            return;
        }
        if (lpparam.packageName.contains(config.analyse_packet_name)) {
            log.i("analyse inject process " + lpparam.processName + "!");
//            tools.add(new tool_okhttp_analyse(config, lpparam));
//            analyses.add(new bugly_analyse(config, lpparam));
//            tools.add(new tool_sslunpinning(config, lpparam));
//            tools.add(new tool_inject_jar(config, lpparam));
//            tools.add(new sub_plug_txsd(config, lpparam));
            tools.add(new tool_pass_root(config, lpparam));

//            XposedHelpers.findAndHookMethod(
//                    "com.stub.StubApp",
//                    lpparam.classLoader,
//                    "a",
//                    Context.class,
//                    new XC_MethodHook() {
//                        @Override
//                        protected void afterHookedMethod(MethodHookParam param) throws Throwable {
//                            super.afterHookedMethod(param);
//                            //获取到360的Context对象，通过这个对象来获取classloader
//                            Context context = (Context) param.args[0];
//                            //获取360的classloader，之后hook加固后的就使用这个classloader
//                            ClassLoader class_loader = context.getClassLoader();
//                            log.i("find class loader: " + class_loader.toString());
//                            for (base_analyse analyse : analyses) {
//                                analyse.on_360_reinforce(context, class_loader);
//                            }
//                        }
//                    }
//            );

//            XposedHelpers.findAndHookMethod(ClassLoader.class, "loadClass", String.class, new XC_MethodHook() {
//                @Override
//                protected void afterHookedMethod(MethodHookParam param) {
//                    if (param.hasThrowable()) return;
//                    if (param.args.length != 1) return;
//
//                    Class<?> cls = (Class<?>) param.getResult();
//                    ClassLoader class_loader = cls.getClassLoader();
//                    String class_name = (String) param.args[0];
//                    log.i("------load " + class_name);
//                    for (base_analyse analyse : analyses) {
//                        analyse.on_loadclass(class_name, cls, class_loader, param);
//                    }
//                }
//            });

            Class ActivityThread = XposedHelpers.findClass("android.app.ActivityThread", lpparam.classLoader);
            XposedBridge.hookAllMethods(ActivityThread, "performLaunchActivity", new XC_MethodHook() {
                @Override
                protected void afterHookedMethod(MethodHookParam param) throws Throwable {
                    super.afterHookedMethod(param);
                    Application mInitialApplication = (Application) XposedHelpers.getObjectField(param.thisObject, "mInitialApplication");
                    ClassLoader class_loader = (ClassLoader) XposedHelpers.callMethod(mInitialApplication, "getClassLoader");
                    log.i("find class loader: " + class_loader.toString());
                    init_plug(xposed_plug_common.plug_name.plug_name_sub_plug_loader, mInitialApplication.getApplicationContext());
                    for (sub_plug_base analyse : tools) {
                        try {
                            analyse.on_application_class_loader(mInitialApplication, class_loader);
                            log.i("on_application_class_loader " + analyse.name);
                        } catch (Exception | Error e) {
                            log.i("on_application_class_loader error!" + e);
                        }
                    }
                }
            });

        }
    }
}
