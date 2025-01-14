#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <android/log.h>

#include "module.h"
#include "zygisk.hpp"

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

// Spoofing apps
static std::vector<std::string> P1 = {"com.samsung.android.rampart"};
static std::vector<std::string> P5 = {"com.sec.android.app.samsungapps"};
static std::vector<std::string> P6 = {"com.samsung.android.smartsuggestions"};
static std::vector<std::string> P7 = {"com.google.android.apps.messaging"};
static std::vector<std::string> P8 = {};
static std::vector<std::string> PFold = {};
static std::vector<std::string> keep = {};

// Fingerprint
const char P1_FP[256] = "samsung/dm3qxxx/qssi:14/UP1A.231005.007/S918BXXU3BWJM:user/release-keys";
const char P5_FP[256] = "samsung/gts9uxxx/qssi:13/TP1A.220624.014/X916BXXU1AWG1:user/release-keys";
const char P6_FP[256] = "samsung/gts9uxxx/qssi:13/TP1A.220624.014/X916BXXU1AWG1:user/release-keys";
const char P7_FP[256] = "samsung/dm3qxxx/qssi:14/UP1A.231005.007/S918BXXU3CXCF:user/release-keys";
const char P2_FP[256] = "samsung/gts9uxxx/qssi:13/TP1A.220624.014/X916BXXU1AWG1:user/release-keys";
const char PF_FP[256] = "samsung/gts9uxxx/qssi:13/TP1A.220624.014/X916BXXU1AWG1:user/release-keys";
const char P8_FP[256] = "samsung/gts9uxxx/qssi:13/TP1A.220624.014/X916BXXU1AWG1:user/release-keys";


bool DEBUG = true;
char package_name[256];
static int spoof_type;

class onecompleter : public zygisk::ModuleBase
{
public:
    void onLoad(Api *api, JNIEnv *env) override
    {
        this->api = api;
        this->env = env;
    }
    void preAppSpecialize(AppSpecializeArgs *args) override
    {
        const char *process = env->GetStringUTFChars(args->nice_name, nullptr);
        spoof_type = getSpoof(process);
        strcpy(package_name, process);
        env->ReleaseStringUTFChars(args->nice_name, process);
    }
    void postAppSpecialize(const AppSpecializeArgs *) override
    {
        switch (spoof_type)
        {
        case 1:
            injectBuild("SM-S918B", "dm3qxxx", P1_FP);
            injectversion(34);
            break;
        case 2:
            injectBuild("SM-X916B", "gts9uxxx", P5_FP);
            break;
        case 3:
            injectBuild("SM-X916B", "gts9uxxx", P6_FP);
            break;
        case 4:
            injectBuild("SM-S918B", "dm3qxxx", P7_FP);
            break;
        case 5:
            injectBuild("SM-X916B", "gts9uxxx", P1_FP);
            break;
        case 6:
            injectBuild("SM-X916B", "gts9uxxx", P7_FP);
            break;
        case 7:
            injectBuild("SM-X916B", "gts9uxxx", P8_FP);
            break;
        default:
            break;
        }
    }

private:
    Api *api;
    JNIEnv *env;

    void injectBuild(const char *model1, const char *product1, const char *finger1)
    {
        if (env == nullptr)
        {
            LOGW("failed to inject android.os.Build for %s due to env is null", package_name);
            return;
        }

        jclass build_class = env->FindClass("android/os/Build");
        if (build_class == nullptr)
        {
            LOGW("failed to inject android.os.Build for %s due to build is null", package_name);
            return;
        }
        else if (DEBUG)
        {
            LOGI("inject android.os.Build for %s with \nPRODUCT:%s \nMODEL:%s \nFINGERPRINT:%s", package_name, product1, model1, finger1);
        }

        jstring product = env->NewStringUTF(product1);
        jstring model = env->NewStringUTF(model1);
        jstring brand = env->NewStringUTF("samsung");
        jstring manufacturer = env->NewStringUTF("samsung");
        jstring finger = env->NewStringUTF(finger1);
        jstring tag = env->NewStringUTF("release-keys");
        jstring type = env->NewStringUTF("user");

        jfieldID brand_id = env->GetStaticFieldID(build_class, "BRAND", "Ljava/lang/String;");
        if (brand_id != nullptr)
        {
            env->SetStaticObjectField(build_class, brand_id, brand);
        }
        jfieldID manufacturer_id = env->GetStaticFieldID(build_class, "MANUFACTURER", "Ljava/lang/String;");
        if (manufacturer_id != nullptr)
        {
            env->SetStaticObjectField(build_class, manufacturer_id, manufacturer);
        }
        jfieldID product_id = env->GetStaticFieldID(build_class, "PRODUCT", "Ljava/lang/String;");
        if (product_id != nullptr)
        {
            env->SetStaticObjectField(build_class, product_id, product);
        }
        jfieldID device_id = env->GetStaticFieldID(build_class, "DEVICE", "Ljava/lang/String;");
        if (device_id != nullptr)
        {
            env->SetStaticObjectField(build_class, device_id, product);
        }
        jfieldID model_id = env->GetStaticFieldID(build_class, "MODEL", "Ljava/lang/String;");
        if (model_id != nullptr)
        {
            env->SetStaticObjectField(build_class, model_id, model);
        }
        jfieldID tag_id = env->GetStaticFieldID(build_class, "TAGS", "Ljava/lang/String;");
        if (tag_id != nullptr)
        {
            env->SetStaticObjectField(build_class, tag_id, tag);
        }
        jfieldID type_id = env->GetStaticFieldID(build_class, "TYPE", "Ljava/lang/String;");
        if (type_id != nullptr)
        {
            env->SetStaticObjectField(build_class, type_id, type);
        }
        jfieldID finger_id = env->GetStaticFieldID(build_class, "FINGERPRINT", "Ljava/lang/String;");
        if (finger_id != nullptr)
        {
            env->SetStaticObjectField(build_class, finger_id, finger);
        }

        if (env->ExceptionCheck())
        {
            env->ExceptionClear();
        }

        env->DeleteLocalRef(brand);
        env->DeleteLocalRef(manufacturer);
        env->DeleteLocalRef(product);
        env->DeleteLocalRef(model);
        env->DeleteLocalRef(type);
        env->DeleteLocalRef(tag);
        env->DeleteLocalRef(finger);
    }
    void injectsoc(const char *soc)
    {
        if (env == nullptr)
        {
            LOGW("failed to inject android.os.Build for %s due to env is null", package_name);
            return;
        }

        jclass build_class = env->FindClass("android/os/Build");
        if (build_class == nullptr)
        {
            LOGW("failed to inject android.os.Build for %s due to build is null", package_name);
            return;
        }

        jstring inc = env->NewStringUTF(soc);

        jfieldID inc_id = env->GetStaticFieldID(build_class, "SOC_MODEL", "Ljava/lang/String;");
        if (inc_id != nullptr)
        {
            env->SetStaticObjectField(build_class, inc_id, inc);
        }

        if (env->ExceptionCheck())
        {
            env->ExceptionClear();
        }
    }
    void injecthardware(const char *hd)
    {
        if (env == nullptr)
        {
            LOGW("failed to inject android.os.Build for %s due to env is null", package_name);
            return;
        }

        jclass build_class = env->FindClass("android/os/Build");
        if (build_class == nullptr)
        {
            LOGW("failed to inject android.os.Build for %s due to build is null", package_name);
            return;
        }

        jstring inc = env->NewStringUTF(hd);

        jfieldID inc_id = env->GetStaticFieldID(build_class, "HARDWARE", "Ljava/lang/String;");
        if (inc_id != nullptr)
        {
            env->SetStaticObjectField(build_class, inc_id, inc);
        }

        if (env->ExceptionCheck())
        {
            env->ExceptionClear();
        }
    }
    void injectversion(const int inc_c)
    {
        if (env == nullptr)
        {
            LOGW("failed to inject android.os.Build for %s due to env is null", package_name);
            return;
        }

        jclass build_class = env->FindClass("android/os/Build$VERSION");
        if (build_class == nullptr)
        {
            LOGW("failed to inject android.os.Build.VERSION for %s due to build is null", package_name);
            return;
        }

        jint inc = (jint)inc_c;

        jfieldID inc_id = env->GetStaticFieldID(build_class, "SDK_INT", "I");
        if (inc_id != nullptr)
        {
            env->SetStaticIntField(build_class, inc_id, inc);
        }

        if (env->ExceptionCheck())
        {
            env->ExceptionClear();
        }
    }    
    int getSpoof(const char *process)
    {
        std::string package = process;

        for (auto &s : keep)
        {
            if (package.find(s) != std::string::npos)
                return 0;
        }
        for (auto &s : P1)
        {
            if (package.find(s) != std::string::npos)
                return 1;
        }
        for (auto &s : P8)
        {
            if (package.find(s) != std::string::npos)
                return 7;
        }
        for (auto &s : P5)
        {
            if (package.find(s) != std::string::npos)
                return 3;
        }
        for (auto &s : P7)
        {
            if (package.find(s) != std::string::npos)
                return 5;
        }
        for (auto &s : PFold)
        {
            if (package.find(s) != std::string::npos)
                return 6;
        }
        for (auto &s : P6)
        {
            if (package.find(s) != std::string::npos)
                return 4;
        }
        return 0;
    }
};

REGISTER_ZYGISK_MODULE(onecompleter)
