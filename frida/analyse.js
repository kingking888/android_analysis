var target_name = 'libpjsua2.so'

function inject_jnitrace() {
    var so_path = ''
    var hook_so
    if (Process.arch === "arm64") {
        so_path = "/data/app/libnative_hook_64.so"
    } else if (Process.arch === "arm") {
        so_path = "/data/app/libnative_hook_32.so"
    }

    hook_so = Module.load(so_path)

    console.log("jnitrace.so: " + so_path)
    let init_hook_addr = hook_so.findExportByName('init_jni_hook')
    console.log("init_jni_hook: " + init_hook_addr)
    var init_hook = new NativeFunction(init_hook_addr, 'void', ['pointer', 'pointer'])

    let so_path_native_addr = Memory.alloc(target_name.length + 2)
    so_path_native_addr.writeUtf8String(target_name)
    console.log("target module: " + so_path_native_addr.readCString())

    let jnienv = Java.vm.getEnv().handle

    init_hook(jnienv, so_path_native_addr)
    console.log("finish hook!")
}

function hook_android_dlopen_ext(modeul_name) {
    var android_dlopen_ext = Module.findExportByName(null, "android_dlopen_ext")
    console.log(" android_dlopen_ext:" + android_dlopen_ext);
    Interceptor.attach(android_dlopen_ext, {
        onEnter: function (args) {
            this.file_name = args[0].readCString();
        },
        onLeave: function (retval) {
            if (this.file_name.indexOf(modeul_name) >= 0) {
                console.log("loaded : " + this.file_name)
                inject_jnitrace()
            }
        },
    })
}

// frida -U -f com.whatsapp -l analyse.js --no-pause

//frida -U -f com.view.asim -l analyse.js --no-pause
hook_android_dlopen_ext(target_name)