//#include <jni.h>
//#include <android/log.h>
//#include <string>
//#include <map>
//#include <atomic>
//
//
//#include "utils/log.h"
//#include "utils/utils.h"
//#include "utils/str_helper.h"
//#include "utils/jni_helper.h"
//#include "utils/java_vm.h"
//
//#include "libxhook/xhook.h"
//#include "dlfc/dlfcn_nougat.h"
//#include "hook_libc/libc_hook_export.h"
////#include "hook_jni/enum_jni_static_method.h"
////#include "hook_jni/jni_env_hook.h"
//#include "hook_jni/jni_hook.h"
//
//using namespace std;
//
//class boot_id_factory : public filter_factory {
//    string boot_id;
//public:
//    explicit boot_id_factory(const string &boot_id) {
//        this->boot_id = boot_id;
//    }
//
//    file_filter *create() override {
//        return new memory_file(boot_id.c_str(), boot_id.length());
//    }
//};
//
//
////hook_file_info hook_fopen_list[] = {
////  {	"/proc/sys/kernel/random/uuid",	false,0},
////  {	"/proc/self/maps",				false,1},
////  {	"/proc/net/arp",				false,2},
////  {	"/proc/iomem",					false,3},
////  {	"/proc/misc",					false,4},
////  {	"/proc/self/mounts",			false,5},
////  {	"/proc/interrupts",				false,6},
////  {	"/proc/ioports",				false,7},
////  {	"/proc/kallsyms",				false,8},
////  {	"/proc/net/route",				false,9}
////};
//
////hook_file_info hook_popen_list[] = {
////  {	"/proc/sys/kernel/random/boot_id",	false,0}
////};
////
////hook_file_info hook_open_list[] = {
////  {	"/system/bin/ls",	false,0}
////};
//
//using func_JNI_OnLoad = jint (*)(JavaVM *vm, void *reserved);
//using func_OnUnLoad = void (*)(JavaVM *jvm, void *reserved);
//
//struct shumei_handle {
//    void *lib_handle;
//    func_JNI_OnLoad OnLoad;
//    func_OnUnLoad OnUnLoad;
//    shumei_config *config;
//    vector<int> file_hook_list;
//};
//
//JavaVM *g_vm;
//void *g_reserved;
//
//map<jint, shumei_handle *> g_map_handle;
//atomic<jint> g_handle_index(0);
//
//
//extern "C"
//JNIEXPORT jint JNICALL
//Java_com_hook_native_1hook_1helper_jload_1smsdk(JNIEnv *env, jobject thiz, jstring config) {
//    // TODO: implement jload_smsdk()
//    auto *handle = new shumei_handle;
//    handle->lib_handle = dlopen("libsmsdk.so", RTLD_LAZY);
//    init_jni_hook(env, "libsmsdk.so");
//
//    elf_info *fake_handle = fake_dlopen("/data/app|libsmsdk.so", RTLD_LAZY);
//    handle->OnLoad = (func_JNI_OnLoad) fake_dlsym(fake_handle, "JNI_OnLoad");
//    handle->OnUnLoad = (func_OnUnLoad) fake_dlsym(fake_handle, "JNI_OnUnLoad");
//
//    auto *data = new shumei_config(jstring2str(env, config));
//    handle->config = data;
//
////    hook_global_init();
////    init_file_hook();
////    init_only_watch("libsmsdk.so");
//
////    hook_params info;
////    info.reg_path = "cat /proc/sys/kernel/random/boot_id";
////    info.ffactory = new boot_id_factory(handle->config->boot_id);
////    handle->file_hook_list.push_back(register_file_hook(info));
////
////    info.reg_path = "/system/bin/su";
////    info.ffactory = new none_file_factory();
////    handle->file_hook_list.push_back(register_file_hook(info));
////
////    info.reg_path = "/system/xbin/su";
////    info.ffactory = new none_file_factory();
////    handle->file_hook_list.push_back(register_file_hook(info));
////
////    info.reg_path = "/sbin/su";
////    info.ffactory = new none_file_factory();
////    handle->file_hook_list.push_back(register_file_hook(info));
//
//    fake_dlclose(fake_handle);
//    handle->OnLoad(g_vm, g_reserved);
//    jint mhandle = g_handle_index.fetch_add(1);
//    g_map_handle.insert(pair<int, shumei_handle *>(mhandle, handle));
//    return mhandle;
//}
//
//extern "C"
//JNIEXPORT void JNICALL
//Java_com_hook_native_1hook_1helper_junload_1smsdk(JNIEnv *env, jclass clazz, jint mhandle) {
//    // TODO: implement junload_smsdk()
//    shumei_handle *p = g_map_handle.find(mhandle)->second;
//    delete p->config;
//
//    for (auto i :p->file_hook_list) {
//        unregister_file_hook(i);
//    }
//    xhook_clear();
//
//    if (p->OnUnLoad)
//        p->OnUnLoad(g_vm, g_reserved);
//    g_map_handle.erase(mhandle);
//    dlclose(p->lib_handle);
//    delete p;
//}
//
//extern "C"
//JNIEXPORT jstring JNICALL
//Java_com_hook_native_1hook_1helper_jget_1shumei_1config(JNIEnv *env, jclass clazz, jint mhandle) {
//    // TODO: implement jget_shumei_config()
//    shumei_handle *p = g_map_handle.find(mhandle)->second;
//    return string2jstring(env, p->config->to_json());
//}
//
///*
//boolean	    Z
//byte	    B
//char	    C
//short	    S
//int	        I
//long	    J
//float	    F
//double	    D
//String	    L/java/lang/String;
//int[]	    [I
//Object[]	[L/java/lang/Object;
// */
////
////extern "C" void _init(void) {
////    JavaVM *jvm = nullptr;
////    jsize ret;
//////    JNI_GetCreatedJavaVMs(&jvm, 1, &ret);
////    if (ret <= 1) {
////        loge("%s", "get jvm err!");
////    }
////    auto *handle = new shumei_handle;
////    handle->lib_handle = dlopen("libsmsdk.so", RTLD_LAZY);
////    elf_info *fake_handle = fake_dlopen("/data/app|libsmsdk.so", RTLD_LAZY);
////    handle->OnLoad = (func_JNI_OnLoad) fake_dlsym(fake_handle, "JNI_OnLoad");
////    handle->OnUnLoad = (func_OnUnLoad) fake_dlsym(fake_handle, "JNI_OnUnLoad");
//////    handle->OnLoad(get_jni_env(), g_reserved);
////    fake_dlclose(fake_handle);
////
////    hook_global_init();
////    init_file_hook();
////    init_other_hook();
////}
//
////JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
////    g_vm = vm;
////    g_reserved = reserved;
////    JNIEnv *env;
////    vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
//////    init_global_hook_jni_env(env);
////    return JNI_VERSION_1_6;
////}
//
////JNIEXPORT void JNICALL
////JNI_OnUnLoad(JavaVM *jvm, void *reserved) {
////
////}
