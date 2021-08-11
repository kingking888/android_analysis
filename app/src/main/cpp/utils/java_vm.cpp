//#include "java_vm.h" 
//#include <string.h>
////#pragma comment(lib,"libjvm.so")
////#pragma comment(lib,"libverify.so")
////#pragma comment(lib,"libjava.so")
////#pragma comment(lib,"jawt.lib")
//



#include <jni.h>
#include "../dlfc/dlfcn_nougat.h"

JNIEnv *g_tls_jnienv = nullptr;

JNIEnv *get_jni_env() {
    if (!g_tls_jnienv) {
        elf_info *dlctx = fake_dlopen("libandroid_runtime.so", 0);
        if (dlctx) {
            typedef JNIEnv *(*getJNIEnv_t)();
            getJNIEnv_t getJNIEnv = (getJNIEnv_t) fake_dlsym(dlctx,
                                                             "_ZN7android14AndroidRuntime9getJNIEnvEv");
            if (getJNIEnv)
                g_tls_jnienv = getJNIEnv();
            fake_dlclose(dlctx);
        }
    }
    return g_tls_jnienv;
}


//extern "C" {
//	JavaVM* jvm = NULL;
//	JNIEnv* env = NULL;
//
//	JNIEnv* get_java_env() {
//
//		return env;
//	}
//
//	int create_java_env(JavaVM** jvm, JNIEnv** env)
//	{
//		/*
//		接下来，声明所有希望在程序中使用的变量。
//		JavaVMOption options[] 具有用于 JVM 的各种选项设置。
//		当声明变量时，确保所声明的JavaVMOption options[] 数组足够大，以便能容纳您希望使用的所有选项。
//		在本例中，我们使用的唯一选项就是类路径选项。
//		因为在本示例中，我们所有的文件都在同一目录中，所以将类路径设置成当前目录。
//		可以设置类路径，使它指向任何您希望使用的目录结构。*/
//		JavaVMOption options[2];
//		JavaVMInitArgs vm_args;
//		/*JNIEnv *env          表示 JNI 执行环境。
//		JavaVM jvm             是指向 JVM 的指针,我们主要使用这个指针来创建、初始化和销毁 JVM。
//		JavaVMInitArgs vm_args 表示可以用来初始化 JVM 的各种 JVM 参数。*/
//
//		long status;
//
//		/*avaVMInitArgs 结构表示用于 JVM 的初始化参数。
//		在执行 Java 代码之前，可以使用这些参数来定制运行时环境。
//		正如您所见，这些选项是一个参数,而 Java 版本是另一个参数。
//		按如下所示设置了这些参数：*/
//
//		/*为 JVM 设置类路径，以使它能找到所需要的 Java 类。
//		在这个特定示例中，因为 Sample2.class 和Sample2.exe 都位于同一目录中，所以将类路径设置成当前目录。
//		我们用来为 Sample2.c 设置类路径的代码如下所示：*/
//		options[0].optionString = "-Djava.class.path=.";
//		//options[1].optionString = "-Djava.library.path=C:\\Program Files (x86)\\Java\\jdk1.8.0_281\\jre\\bin";
//		memset(&vm_args, 0, sizeof(vm_args));
//		vm_args.version = JNI_VERSION_1_2;
//		vm_args.nOptions = 1;
//		vm_args.options = options;
//
//		/*创建 JVM
//		处理完所有设置之后，现在就准备创建 JVM 了。先从调用方法开始
//		如果成功，则这个方法返回零，否则，如果无法创建 JVM，则返回JNI_ERR。*/
//		status = JNI_CreateJavaVM(jvm, (void**)env, &vm_args);
//		return status != JNI_ERR;
//	}
//
//	void delete_java_env(JavaVM* jvm) {
//		(*jvm)->DestroyJavaVM(jvm);
//	}
//
//	int init_java() {
//		return create_java_env(&jvm, &env);
//	}
//
//	void uninit_java() {
//		delete_java_env(jvm);
//	}
//
//
//
//}
// 