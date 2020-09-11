/*++

    Copyright (c) Microsoft Corporation.
    Licensed under the MIT License.

Abstract:

    This file contains helpers for using MsQuic.

Environment:

    user mode or kernel mode

--*/

#pragma once

#include <quic_platform.h>
#include <msquic.h>
#include <msquicp.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct QUIC_CREDENTIAL_CONFIG_HELPER {
    QUIC_CREDENTIAL_CONFIG CredConfig;
    union {
        QUIC_CERTIFICATE_HASH CertHash;
        QUIC_CERTIFICATE_HASH_STORE CertHashStore;
        QUIC_CERTIFICATE_FILE CertFile;
    };
} QUIC_CREDENTIAL_CONFIG_HELPER;

//
// Converts the QUIC Status Code to a string for console output.
//
inline
_Null_terminated_
const char*
QuicStatusToString(
    _In_ QUIC_STATUS Status
    )
{
    switch (Status) {
    case QUIC_STATUS_SUCCESS:                   return "SUCCESS";
    case QUIC_STATUS_OUT_OF_MEMORY:             return "OUT_OF_MEMORY";
    case QUIC_STATUS_INVALID_PARAMETER:         return "INVALID_PARAMETER";
    case QUIC_STATUS_INVALID_STATE:             return "INVALID_STATE";
    case QUIC_STATUS_NOT_SUPPORTED:             return "NOT_SUPPORTED";
    case QUIC_STATUS_NOT_FOUND:                 return "NOT_FOUND";
    case QUIC_STATUS_BUFFER_TOO_SMALL:          return "BUFFER_TOO_SMALL";
    case QUIC_STATUS_HANDSHAKE_FAILURE:         return "HANDSHAKE_FAILURE";
    case QUIC_STATUS_ABORTED:                   return "ABORTED";
    case QUIC_STATUS_ADDRESS_IN_USE:            return "ADDRESS_IN_USE";
    case QUIC_STATUS_CONNECTION_TIMEOUT:        return "CONNECTION_TIMEOUT";
    case QUIC_STATUS_CONNECTION_IDLE:           return "CONNECTION_IDLE";
    case QUIC_STATUS_UNREACHABLE:               return "UNREACHABLE";
    case QUIC_STATUS_INTERNAL_ERROR:            return "INTERNAL_ERROR";
    case QUIC_STATUS_CONNECTION_REFUSED:        return "CONNECTION_REFUSED";
    case QUIC_STATUS_PROTOCOL_ERROR:            return "PROTOCOL_ERROR";
    case QUIC_STATUS_VER_NEG_ERROR:             return "VER_NEG_ERROR";
    case QUIC_STATUS_PENDING:                   return "PENDING";
    }

    return "UNKNOWN";
}

//
// Helper function to get the RTT (in microseconds) from a MsQuic Connection or Stream handle.
//
inline
uint32_t
GetConnRtt(
    _In_ const QUIC_API_TABLE* MsQuic,
    _In_ HQUIC Handle
    )
{
    QUIC_STATISTICS Value;
    uint32_t ValueSize = sizeof(Value);
    MsQuic->GetParam(
        Handle,
        QUIC_PARAM_LEVEL_CONNECTION,
        QUIC_PARAM_CONN_STATISTICS,
        &ValueSize,
        &Value);
    return Value.Rtt;
}

//
// Helper function to get the Stream ID from a MsQuic Stream handle.
//
inline
uint64_t
GetStreamID(
    _In_ const QUIC_API_TABLE* MsQuic,
    _In_ HQUIC Handle
    )
{
    uint64_t ID = (uint32_t)(-1);
    uint32_t IDLen = sizeof(ID);
    MsQuic->GetParam(
        Handle,
        QUIC_PARAM_LEVEL_STREAM,
        QUIC_PARAM_STREAM_ID,
        &IDLen,
        &ID);
    return ID;
}

//
// Helper function to get the remote IP address (as a string) from a MsQuic
// Connection or Stream handle.
//
inline
QUIC_ADDR_STR
GetRemoteAddr(
    _In_ const QUIC_API_TABLE* MsQuic,
    _In_ HQUIC Handle
    )
{
    QUIC_ADDR addr;
    uint32_t addrLen = sizeof(addr);
    QUIC_ADDR_STR addrStr = { 0 };
    QUIC_STATUS status =
        MsQuic->GetParam(
            Handle,
            QUIC_PARAM_LEVEL_CONNECTION,
            QUIC_PARAM_CONN_REMOTE_ADDRESS,
            &addrLen,
            &addr);
    if (QUIC_SUCCEEDED(status)) {
        QuicAddrToString(&addr, &addrStr);
    }
    return addrStr;
}

inline
QUIC_STATUS
QuicForceRetry(
    _In_ const QUIC_API_TABLE* MsQuic,
    _In_ BOOLEAN Enabled
    )
{
    uint16_t value = Enabled ? 0 : 65;
    return
        MsQuic->SetParam(
            NULL,
            QUIC_PARAM_LEVEL_GLOBAL,
            QUIC_PARAM_GLOBAL_RETRY_MEMORY_PERCENT,
            sizeof(value),
            &value);
}

inline
void
DumpMsQuicPerfCounters(
    _In_ const QUIC_API_TABLE* MsQuic
    )
{
    uint64_t Counters[QUIC_PERF_COUNTER_MAX] = {0};
    uint32_t Lenth = sizeof(Counters);
    MsQuic->GetParam(
        NULL,
        QUIC_PARAM_LEVEL_GLOBAL,
        QUIC_PARAM_GLOBAL_PERF_COUNTERS,
        &Lenth,
        &Counters);
    printf("Perf Counters:\n");
    printf("  CONN_CREATED:          %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_CONN_CREATED]);
    printf("  CONN_HANDSHAKE_FAIL:   %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_CONN_HANDSHAKE_FAIL]);
    printf("  CONN_APP_REJECT:       %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_CONN_APP_REJECT]);
    printf("  CONN_ACTIVE:           %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_CONN_ACTIVE]);
    printf("  CONN_CONNECTED:        %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_CONN_CONNECTED]);
    printf("  CONN_PROTOCOL_ERRORS:  %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_CONN_PROTOCOL_ERRORS]);
    printf("  CONN_NO_ALPN:          %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_CONN_NO_ALPN]);
    printf("  STRM_ACTIVE:           %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_STRM_ACTIVE]);
    printf("  PKTS_SUSPECTED_LOST:   %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_PKTS_SUSPECTED_LOST]);
    printf("  PKTS_DROPPED:          %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_PKTS_DROPPED]);
    printf("  PKTS_DECRYPTION_FAIL:  %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_PKTS_DECRYPTION_FAIL]);
    printf("  UDP_RECV:              %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_UDP_RECV]);
    printf("  UDP_SEND:              %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_UDP_SEND]);
    printf("  UDP_RECV_BYTES:        %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_UDP_RECV_BYTES]);
    printf("  UDP_SEND_BYTES:        %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_UDP_SEND_BYTES]);
    printf("  UDP_RECV_EVENTS:       %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_UDP_RECV_EVENTS]);
    printf("  UDP_SEND_CALLS:        %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_UDP_SEND_CALLS]);
    printf("  APP_SEND_BYTES:        %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_APP_SEND_BYTES]);
    printf("  APP_RECV_BYTES:        %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_APP_RECV_BYTES]);
    printf("  CONN_QUEUE_DEPTH:      %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_CONN_QUEUE_DEPTH]);
    printf("  CONN_OPER_QUEUE_DEPTH: %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_CONN_OPER_QUEUE_DEPTH]);
    printf("  CONN_OPER_QUEUED:      %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_CONN_OPER_QUEUED]);
    printf("  CONN_OPER_COMPLETED:   %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_CONN_OPER_COMPLETED]);
    printf("  WORK_OPER_QUEUE_DEPTH: %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_WORK_OPER_QUEUE_DEPTH]);
    printf("  WORK_OPER_QUEUED:      %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_WORK_OPER_QUEUED]);
    printf("  WORK_OPER_COMPLETED:   %llu\n", (unsigned long long)Counters[QUIC_PERF_COUNTER_WORK_OPER_COMPLETED]);
}

//
// Converts an input command line arg string and port to a socket address.
// Supports IPv4, IPv6 or '*' input strings.
//
inline
BOOLEAN
ConvertArgToAddress(
    _In_z_ const char* Arg,
    _In_ uint16_t Port,   // Host Byte Order
    _Out_ QUIC_ADDR* Address
    )
{
    if (strcmp("*", Arg) == 0) {
        //
        // Explicitly zero, otherwise kernel mode errors
        //
        QuicZeroMemory(Address, sizeof(*Address));
        QuicAddrSetFamily(Address, AF_UNSPEC);
        QuicAddrSetPort(Address, Port);
        return TRUE;
    }
    return QuicAddrFromString(Arg, Port, Address);
}

inline uint8_t DecodeHexChar(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return 10 + c - 'A';
    if (c >= 'a' && c <= 'f') return 10 + c - 'a';
    return 0;
}

inline
uint32_t
DecodeHexBuffer(
    _In_z_ const char* HexBuffer,
    _In_ uint32_t OutBufferLen,
    _Out_writes_to_(OutBufferLen, return)
        uint8_t* OutBuffer
    )
{
    uint32_t HexBufferLen = (uint32_t)strlen(HexBuffer) / 2;
    if (HexBufferLen > OutBufferLen) {
        return 0;
    }

    for (uint32_t i = 0; i < HexBufferLen; i++) {
        OutBuffer[i] =
            (DecodeHexChar(HexBuffer[i * 2]) << 4) |
            DecodeHexChar(HexBuffer[i * 2 + 1]);
    }

    return HexBufferLen;
}

#if defined(__cplusplus)

//
// Arg Value Parsers
//

inline
bool
IsArg(
    _In_z_ const char* Arg,
    _In_z_ const char* toTestAgainst
    )
{
    return Arg[0] && (_strnicmp(Arg + 1, toTestAgainst, strlen(toTestAgainst)) == 0);
}

inline
bool
IsValue(
    _In_z_ const char* name,
    _In_z_ const char* toTestAgainst
    )
{
    return _strnicmp(name, toTestAgainst, min(strlen(name), strlen(toTestAgainst))) == 0;
}

//
// Helper function that searches the list of args for a given
// parameter name, insensitive to case.
//
inline
_Ret_maybenull_ _Null_terminated_ const char*
GetValue(
    _In_ int argc,
    _In_reads_(argc) _Null_terminated_ char* argv[],
    _In_z_ const char* name
    )
{
    const size_t nameLen = strlen(name);
    for (int i = 0; i < argc; i++) {
        if (_strnicmp(argv[i] + 1, name, nameLen) == 0) {
            return argv[i] + 1 + nameLen + 1;
        }
    }
    return nullptr;
}

inline
_Success_(return != false)
bool
TryGetValue(
    _In_ int argc,
    _In_reads_(argc) _Null_terminated_ char* argv[],
    _In_z_ const char* name,
    _Out_ _Null_terminated_ const char** pValue
    )
{
    auto value = GetValue(argc, argv, name);
    if (!value) return false;
    *pValue = value;
    return true;
}

inline
_Success_(return != false)
bool
TryGetValue(
    _In_ int argc,
    _In_reads_(argc) _Null_terminated_ char* argv[],
    _In_z_ const char* name,
    _Out_ uint8_t* pValue
    )
{
    auto value = GetValue(argc, argv, name);
    if (!value) return false;
    *pValue = (uint8_t)atoi(value);
    return true;
}

inline
_Success_(return != false)
bool
TryGetValue(
    _In_ int argc,
    _In_reads_(argc) _Null_terminated_ char* argv[],
    _In_z_ const char* name,
    _Out_ uint16_t* pValue
    )
{
    auto value = GetValue(argc, argv, name);
    if (!value) return false;
    *pValue = (uint16_t)atoi(value);
    return true;
}

inline
_Success_(return != false)
bool
TryGetValue(
    _In_ int argc,
    _In_reads_(argc) _Null_terminated_ char* argv[],
    _In_z_ const char* name,
    _Out_ uint32_t* pValue
    )
{
    auto value = GetValue(argc, argv, name);
    if (!value) return false;
    *pValue = (uint32_t)atoi(value);
    return true;
}

inline
_Success_(return != false)
bool
TryGetValue(
    _In_ int argc,
    _In_reads_(argc) _Null_terminated_ char* argv[],
    _In_z_ const char* name,
    _Out_ uint64_t* pValue
    )
{
    auto value = GetValue(argc, argv, name);
    if (!value) return false;
    char* End;
#ifdef _WIN32
    *pValue = _strtoui64(value, &End, 10);
#else
    *pValue = strtoull(value, &End, 10);
#endif
    return true;
}

inline
_Success_(return != false)
bool
GetServerCredConfigFromArgs(
    _In_ int argc,
    _In_reads_(argc) _Null_terminated_ char* argv[],
    _Out_ QUIC_CREDENTIAL_CONFIG_HELPER* Config
    )
{
    Config->CredConfig.Flags = QUIC_CREDENTIAL_FLAG_NONE;

    const char* Cert;
    const char* KeyFile;
    if ((Cert = GetValue(argc, argv, "thumbprint")) != nullptr ||
        (Cert = GetValue(argc, argv, "cert_hash")) != nullptr ||
        (Cert = GetValue(argc, argv, "hash")) != nullptr) {
        uint32_t CertHashLen =
            DecodeHexBuffer(
                Cert,
                sizeof(Config->CertHash.ShaHash),
                Config->CertHash.ShaHash);
        if (CertHashLen != sizeof(Config->CertHash.ShaHash)) {
            return false;
        }
        Config->CredConfig.Type = QUIC_CREDENTIAL_TYPE_CERTIFICATE_HASH;
        Config->CredConfig.Creds = &Config->CertHash;

    } else if (
        (Cert = GetValue(argc, argv, "file")) != nullptr && (KeyFile = GetValue(argc, argv, "key")) != nullptr ||
        (Cert = GetValue(argc, argv, "cert_file")) != nullptr && (KeyFile = GetValue(argc, argv, "cert_key")) != nullptr) {
        Config->CertFile.CertificateFile = (char*)Cert;
        Config->CertFile.PrivateKeyFile = (char*)KeyFile;
        Config->CredConfig.Type = QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE;
        Config->CredConfig.Creds = &Config->CertFile;

    } else {
        return false;
    }

    return true;
}

#endif
