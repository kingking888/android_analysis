package com.plug.xposed.tool;

import android.annotation.SuppressLint;
import android.app.Application;
import android.content.Context;

import com.plug.base.plug_config;
import com.plug.xposed.base.sub_plug_base;
import com.units.log;

import org.apache.http.conn.scheme.HostNameResolver;
import org.apache.http.conn.ssl.SSLSocketFactory;

import java.net.Socket;
import java.security.KeyStore;
import java.security.SecureRandom;
import java.security.cert.X509Certificate;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.KeyManager;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

import de.robv.android.xposed.XC_MethodHook;
import de.robv.android.xposed.XposedBridge;
import de.robv.android.xposed.XposedHelpers;
import de.robv.android.xposed.callbacks.XC_LoadPackage;

public class tool_sslunpinning extends sub_plug_base {
    public tool_sslunpinning(plug_config config, XC_LoadPackage.LoadPackageParam xposed_param) {
        super(config, xposed_param);
        name = "sslunpinning";
    }

    @Override
    public void on_loadclass(String class_name, Class<?> cls, ClassLoader class_loader, XC_MethodHook.MethodHookParam param) {
        try {
            do_hook(class_loader);
        } catch (Exception e) {
            log.i("sslunpinning error " + e);
        }
    }

    @Override
    public void on_360_reinforce(Context context, ClassLoader class_loader) {
        try {
            do_hook(class_loader);
        } catch (Exception e) {
            log.i("sslunpinning error " + e);
        }
    }

    @Override
    public void on_application_class_loader(Application app, ClassLoader class_loader) {
        try {
            do_hook(class_loader);
        } catch (Exception e) {
            log.i("sslunpinning error " + e);
        }
    }

    public void do_hook(ClassLoader class_loader) {

        // --- Java Secure Socket Extension (JSSE) ---
        //TrustManagerFactory.getTrustManagers >> EmptyTrustManager
        try {
            XposedHelpers.findAndHookMethod("javax.net.ssl.TrustManagerFactory", class_loader, "getTrustManagers", new XC_MethodHook() {
                @Override
                protected void beforeHookedMethod(XC_MethodHook.MethodHookParam param) throws Throwable {

                    TrustManager[] tms = EmptyTrustManager.getInstance();
                    param.setResult(tms);
                }
            });
        } catch (Error e) {
            XposedBridge.log("Unpinning_error: " + e.getMessage());
        }

        //SSLContext.init >> (null,EmptyTrustManager,null)
        try {
            XposedHelpers.findAndHookMethod("javax.net.ssl.SSLContext", class_loader, "init", KeyManager[].class, TrustManager[].class, SecureRandom.class, new XC_MethodHook() {

                @Override
                protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
                    param.args[0] = null;
                    param.args[1] = EmptyTrustManager.getInstance();
                    param.args[2] = null;
                }
            });
        } catch (Error e) {
            XposedBridge.log("Unpinning_error: " + e.getMessage());
        }

        //HttpsURLConnection.setSSLSocketFactory >> new SSLSocketFactory
        try {
            XposedHelpers.findAndHookMethod("javax.net.ssl.HttpsURLConnection", class_loader, "setSSLSocketFactory", javax.net.ssl.SSLSocketFactory.class, new XC_MethodHook() {
                @Override
                protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
                    param.args[0] = XposedHelpers.newInstance(javax.net.ssl.SSLSocketFactory.class);
                }
            });
        } catch (Error e) {
            XposedBridge.log("Unpinning_error: " + e.getMessage());
        }

        try {
            // --- APACHE ---
            Class Scheme = XposedHelpers.findClass("org.apache.http.conn.scheme.Scheme", class_loader);
            //SchemeRegistry.register >> new Scheme
            XposedHelpers.findAndHookMethod("org.apache.http.conn.scheme.SchemeRegistry", class_loader, "register", Scheme, new XC_MethodHook() {
                @Override
                protected void beforeHookedMethod(MethodHookParam param) {
                    if (XposedHelpers.callMethod(param.args[0], "getName").equals("https")) {
                        param.args[0] = XposedHelpers.newInstance(Scheme, "https", SSLSocketFactory.getSocketFactory(), 443);
                    }
                }
            });
        } catch (Error e) {
            XposedBridge.log("Unpinning_error: " + e.getMessage());
        }

        //HttpsURLConnection.setDefaultHostnameVerifier >> SSLSocketFactory.ALLOW_ALL_HOSTNAME_VERIFIER
        try {
            XposedHelpers.findAndHookMethod("org.apache.http.conn.ssl.HttpsURLConnection", class_loader, "setDefaultHostnameVerifier",
                    HostnameVerifier.class, new XC_MethodHook() {
                        @Override
                        protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
                            param.args[0] = SSLSocketFactory.ALLOW_ALL_HOSTNAME_VERIFIER;
                        }
                    });
        } catch (Error e) {
            XposedBridge.log("Unpinning_error: " + e.getMessage());
        }

        //HttpsURLConnection.setHostnameVerifier >> SSLSocketFactory.ALLOW_ALL_HOSTNAME_VERIFIER
        try {
            XposedHelpers.findAndHookMethod("org.apache.http.conn.ssl.HttpsURLConnection", class_loader, "setHostnameVerifier", HostnameVerifier.class,
                    new XC_MethodHook() {
                        @Override
                        protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
                            param.args[0] = SSLSocketFactory.ALLOW_ALL_HOSTNAME_VERIFIER;
                        }
                    });
        } catch (Error e) {
            XposedBridge.log("Unpinning_error: " + e.getMessage());
        }

        //SSLSocketFactory.getSocketFactory >> new SSLSocketFactory
        try {
            XposedHelpers.findAndHookMethod("org.apache.http.conn.ssl.SSLSocketFactory", class_loader, "getSocketFactory", new XC_MethodHook() {
                @Override
                protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
                    param.setResult((SSLSocketFactory) XposedHelpers.newInstance(SSLSocketFactory.class));
                }
            });
        } catch (Error e) {
            XposedBridge.log("Unpinning_error: " + e.getMessage());
        }

        //SSLSocketFactory(...) >> SSLSocketFactory(...){ new EmptyTrustManager()}
        try {
            Class<?> sslSocketFactory = XposedHelpers.findClass("org.apache.http.conn.ssl.SSLSocketFactory", class_loader);
            XposedHelpers.findAndHookConstructor(sslSocketFactory, String.class, KeyStore.class, String.class, KeyStore.class,
                    SecureRandom.class, HostNameResolver.class, new XC_MethodHook() {
                        @Override
                        protected void afterHookedMethod(MethodHookParam param) throws Throwable {

                            String algorithm = (String) param.args[0];
                            KeyStore keystore = (KeyStore) param.args[1];
                            String keystorePassword = (String) param.args[2];
                            SecureRandom random = (SecureRandom) param.args[4];

                            KeyManager[] keymanagers = null;
                            TrustManager[] trustmanagers;

                            if (keystore != null) {
                                keymanagers = (KeyManager[]) XposedHelpers.callStaticMethod(SSLSocketFactory.class, "createKeyManagers", keystore, keystorePassword);
                            }

                            trustmanagers = new TrustManager[]{new EmptyTrustManager()};

                            XposedHelpers.setObjectField(param.thisObject, "sslcontext", SSLContext.getInstance(algorithm));
                            XposedHelpers.callMethod(XposedHelpers.getObjectField(param.thisObject, "sslcontext"), "init", keymanagers, trustmanagers, random);
                            XposedHelpers.setObjectField(param.thisObject, "socketfactory", XposedHelpers.callMethod(XposedHelpers.getObjectField(param.thisObject, "sslcontext"), "getSocketFactory"));
                        }

                    });

        } catch (Error e) {
            XposedBridge.log("Unpinning_error: " + e.getMessage());
        }

        //SSLSocketFactory.isSecure >> true
        try {
            XposedHelpers.findAndHookMethod("org.apache.http.conn.ssl.SSLSocketFactory", class_loader, "isSecure", Socket.class, new XC_MethodHook() {
                @Override
                protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
                    param.setResult(true);
                }
            });
        } catch (Error e) {
            XposedBridge.log("Unpinning_error: " + e.getMessage());
        }

        ///OKHTTP
        try {
            XposedHelpers.findAndHookMethod("okhttp3.CertificatePinner", class_loader, "findMatchingPins", String.class, new XC_MethodHook() {
                protected void beforeHookedMethod(MethodHookParam param) throws Throwable {
                    param.args[0] = "";
                }
            });
        } catch (Error e) {
            XposedBridge.log("Unpinning_error: " + e.getMessage());
        }
    }
}

class EmptyTrustManager implements X509TrustManager {

    private static TrustManager[] emptyTM = null;

    public static TrustManager[] getInstance() {
        if (emptyTM == null) {
            emptyTM = new TrustManager[1];
            emptyTM[0] = new EmptyTrustManager();
        }
        return emptyTM;
    }

    @SuppressLint("TrustAllX509TrustManager")
    @Override
    public void checkClientTrusted(X509Certificate[] chain, String authType) {
    }

    @SuppressLint("TrustAllX509TrustManager")
    @Override
    public void checkServerTrusted(X509Certificate[] chain, String authType) {
    }

    @Override
    public X509Certificate[] getAcceptedIssuers() {
        return new X509Certificate[0];
    }
}
