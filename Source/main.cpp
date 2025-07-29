// C Libraries 
#include <stdbool.h>
#include <string.h>

// Orbis Specific Imports
#include <orbis/Pad.h>
#include <orbis/UserService.h>
#include <orbis/SystemService.h>
#include <orbis/Sysmodule.h>
#include <orbis/Http.h>
#include <orbis/Ssl.h>
#include <orbis/Net.h>

// Pyrite Includes
#include "core.h"
#include "Logging/Public/Log.h"
#include "Kernal/Notification/Public/notification.h"

// Goldhen
#include <Common.h>

#define PLUGIN_NAME "Faith"
#define PLUGIN_DESC "Faith is a request Redirector to redirect traffic from sonys LibSceHttp lib to your own custom server "
#define PLUGIN_AUTH "Tevtongermany"
#define PLUGIN_VER 0x101 // 1.01

DECLARE_LOG_CATEGORY(LogFaith);

const char *RedirectURL = "ENTERURLHERE!!!";



// Hook declarations

// Redirect
HOOK_INIT(sceHttpCreateConnectionWithURL);
HOOK_INIT(sceHttpCreateRequestWithURL);

// Cert Verify dunno if these actually work but Fortnite used one of them so why not hook all of them
HOOK_INIT(CERT_validateCertificate);
HOOK_INIT(CERT_validateCertificateWithConf);
HOOK_INIT(CA_MGMT_verifyCertWithKeyBlob);
HOOK_INIT(CA_MGMT_verifySignature);
HOOK_INIT(CERT_verifySignature);
HOOK_INIT(RSA_verifySignature);
HOOK_INIT(CERT_isRootCertificate);
HOOK_INIT(CERT_isRootCertificate2);


int CERT_validateCertificate_hook(void *cert, void *unused) {
    LOG(LogFaith,LogVerbosity::Log,"Bypassing certificate validation");
    return 0; 
}

int CERT_validateCertificateWithConf_hook(void *cert, void *conf) {
    LOG(LogFaith,LogVerbosity::Log,"Bypassing certificate validation with config");
    return 0;
}

int CA_MGMT_verifyCertWithKeyBlob_hook(void *cert, void *keyBlob) {
    LOG(LogFaith,LogVerbosity::Log,"Bypassing cert verification with key blob");
    return 0;
}

int CA_MGMT_verifySignature_hook(void *cert, void *signature) {
    LOG(LogFaith,LogVerbosity::Log,"Bypassing signature verification");
    return 0;
}

int CERT_verifySignature_hook(void *cert, void *signature) {
    LOG(LogFaith,LogVerbosity::Log,"Bypassing cert signature verification");
    return 1;
}

int RSA_verifySignature_hook(void *cert, void *signature) {
    LOG(LogFaith,LogVerbosity::Log,"Bypassing RSA signature verification");
    return 1;
}

int CERT_isRootCertificate_hook(void *cert) {
    LOG(LogFaith,LogVerbosity::Log,"Forcing root certificate acceptance");
    return 1; 
}

int CERT_isRootCertificate2_hook(void *cert) {
    LOG(LogFaith,LogVerbosity::Log,"Forcing root certificate acceptance (v2)");
    return 1;
}

const char *extract_path(const char *url) {
    if (!url) return NULL;
    const char *start = strstr(url, "://");
    if (!start) return NULL;
    start += 3;
    const char *path = strchr(start, '/');
    return path ? path : "";
}

char *GetUrltoRedirect(const char *url) {
    if (!url) return NULL;

    if (strstr(url, "ol.epicgames.com") != NULL || 
        strstr(url, "epicgames.dev") != NULL ||
        strstr(url, "ol.epicgames.net") != NULL ||
        strstr(url, ".akamaized.net") != NULL ||
        strstr(url, "on.epicgames.com") != NULL ||
        strstr(url, "game-social.epicgames.com") != NULL ||
        strstr(url, "superawesome.com") != NULL || 
        strstr(url, "ak.epicgames.com") != NULL ) 
    {
        const char *Path = extract_path(url);
        if (!Path) return NULL;

        size_t newUrlSize = strlen(RedirectURL) + strlen(Path) + 1;
        char *newRedirectUrl = (char *)malloc(newUrlSize);
        if (!newRedirectUrl) return NULL;

        strcpy(newRedirectUrl, RedirectURL);
        strcat(newRedirectUrl, Path);
        return newRedirectUrl; 
    }

    return NULL;
}




int32_t sceHttpCreateRequestWithURL_hook(int32_t conectId, int32_t method, const char *url, uint64_t contentLength) {
    char * newRedirectUrl = GetUrltoRedirect(url);
    LOG(LogFaith,LogVerbosity::Log,"sceHttp::sceHttpCreateRequestWithURL->Url: %s", url);

    if (newRedirectUrl) {       
        LOG(LogFaith,LogVerbosity::Log,"Redirecting CreateRequest URL:");
        LOG(LogFaith,LogVerbosity::Log,"Original: %s", url);
        LOG(LogFaith,LogVerbosity::Log,"Redirected: %s", newRedirectUrl);
        
        int32_t result = HOOK_CONTINUE(sceHttpCreateRequestWithURL, 
            int32_t(*)(int32_t, int32_t, const char *, uint64_t), conectId, method, newRedirectUrl, contentLength);
        
        free(newRedirectUrl);
        return result;
    } 
    
    return HOOK_CONTINUE(sceHttpCreateRequestWithURL, 
        int32_t(*)(int32_t, int32_t, const char *, uint64_t), 
        conectId, method, url, contentLength);
}

int32_t sceHttpCreateConnectionWithURL_hook(int32_t templateId, const char *url, bool isKeepalive) {
    char * newRedirectUrl = GetUrltoRedirect(url);

    if (newRedirectUrl) {
        
        LOG(LogFaith,LogVerbosity::Log,"Redirecting CreateConnection URL:");
        LOG(LogFaith,LogVerbosity::Log,"Original: %s", url);
        LOG(LogFaith,LogVerbosity::Log,"Redirected: %s", newRedirectUrl);
        
        int32_t result = HOOK_CONTINUE(sceHttpCreateConnectionWithURL, 
            int32_t(*)(int32_t, const char *, bool), 
            templateId, newRedirectUrl, isKeepalive);
        
        free(newRedirectUrl);
        return result;
    }

    return HOOK_CONTINUE(sceHttpCreateConnectionWithURL, 
        int32_t(*)(int32_t, const char *, bool), 
        templateId, url, isKeepalive);
}



extern "C" {
    
    PUBLIC_ATTRIBUTE s32 plugin_load(s32 argc, const char* argv[])  {
        LOG(LogFaith,LogVerbosity::Log,"[GoldHEN] %s Plugin Started.", PLUGIN_NAME);
        LOG(LogFaith,LogVerbosity::Log,"[GoldHEN] Plugin Author(s): %s", PLUGIN_AUTH);

        char* FaithIconUrl = "https://raw.githubusercontent.com/OrbisNet/Faith/refs/heads/main/Art/Faith.png";
		
        NOTIFY(FaithIconUrl,                   "Faith Injected!\n"
                                               "Made by Tevtongermany\n"
                                               "Redirecting to:\n"
                                               "%s",RedirectURL);

        LOG(LogFaith,LogVerbosity::Log,"Hooking into Request Functions");
		
        HOOK32(sceHttpCreateConnectionWithURL);
        HOOK32(sceHttpCreateRequestWithURL);

        LOG(LogFaith,LogVerbosity::Log,"Hooking into Cert validation Functions");
        HOOK32(CERT_validateCertificate);
        HOOK32(CERT_validateCertificateWithConf);
        HOOK32(CA_MGMT_verifyCertWithKeyBlob);
        HOOK32(CA_MGMT_verifySignature);
        HOOK32(CERT_verifySignature);
        HOOK32(RSA_verifySignature);
        HOOK32(CERT_isRootCertificate);
        HOOK32(CERT_isRootCertificate2);
        
        return 0;
    } 


    
    PUBLIC_ATTRIBUTE s32 plugin_unload(s32 argc, const char* argv[]) {
        LOG(LogFaith,LogVerbosity::Log,"[GoldHEN] <%s\\Ver.0x%08x> %s", PLUGIN_NAME, PLUGIN_VER, __func__);
        LOG(LogFaith,LogVerbosity::Log,"[GoldHEN] %s Plugin Ended.", PLUGIN_NAME);

        UNHOOK(sceHttpCreateConnectionWithURL);
        UNHOOK(sceHttpCreateRequestWithURL);
        
        UNHOOK(CERT_validateCertificate);
        UNHOOK(CERT_validateCertificateWithConf);
        UNHOOK(CA_MGMT_verifyCertWithKeyBlob);
        UNHOOK(CA_MGMT_verifySignature);
        UNHOOK(CERT_verifySignature);
        UNHOOK(RSA_verifySignature);
        UNHOOK(CERT_isRootCertificate);
        UNHOOK(CERT_isRootCertificate2);

        return 0;
    }

    
    WEAK_ATTRIBUTE s32 module_start(s64 argc, const void *args) {
        return 0;
    }


    WEAK_ATTRIBUTE s32 module_stop(s64 argc, const void *args) {
        return 0;
    }

} 
