#include "steamclient_private.h"

WINE_DEFAULT_DEBUG_CHANNEL(steamclient);

struct networking_message_pool
{
    LONG ref;
    void *message_data;
    struct networking_message messages[];
};

C_ASSERT( sizeof(struct networking_message_pool) == offsetof( struct networking_message_pool, messages[0] ) );

static void networking_message_pool_release( struct networking_message_pool *pool )
{
    if (!InterlockedDecrement( &pool->ref ))
    {
        HeapFree( GetProcessHeap(), 0, pool->message_data );
        HeapFree( GetProcessHeap(), 0, pool );
    }
}

static BOOL networking_message_pool_alloc_data( uint32_t count, struct networking_message_pool *pool )
{
    uint32_t i, size;
    char *ptr;

    for (i = 0, size = 0; i < count; i++) size += *pool->messages[i].p_size;
    if (!(pool->message_data = HeapAlloc( GetProcessHeap(), 0, size )))
    {
        ERR( "Failed to allocate memory for networking messages\n" );
        return FALSE;
    }

    for (i = 0, ptr = pool->message_data; i < count; i++)
    {
        *pool->messages[i].p_data = ptr;
        ptr += *pool->messages[i].p_size;
    }

    return TRUE;
}

static void W_CDECL w_SteamNetworkingMessage_t_144_FreeData( w_SteamNetworkingMessage_t_144 *msg )
{
    if (msg->m_pData) SecureZeroMemory( msg->m_pData, msg->m_cbSize );
}

static void W_CDECL w_SteamNetworkingMessage_t_144_Release( w_SteamNetworkingMessage_t_144 *msg )
{
    struct networking_message *message = CONTAINING_RECORD( msg, struct networking_message, w_msg_144 );

    if (msg->m_pfnFreeData) msg->m_pfnFreeData( msg );
    SecureZeroMemory( msg, sizeof(*msg) );

    networking_message_pool_release( message->pool );
}

static w_SteamNetworkingMessage_t_144 *networking_message_init_144( struct networking_message *message,
                                                                    struct networking_message_pool *pool )
{
    message->pool = pool;
    message->p_data = &message->w_msg_144.m_pData;
    message->p_size = &message->w_msg_144.m_cbSize;
    message->w_msg_144.m_pfnFreeData = w_SteamNetworkingMessage_t_144_FreeData;
    message->w_msg_144.m_pfnRelease = w_SteamNetworkingMessage_t_144_Release;
    return &message->w_msg_144;
}

static BOOL networking_message_pool_create_144( uint32_t count, w_SteamNetworkingMessage_t_144 **messages )
{
    uint32_t size = offsetof( struct networking_message_pool, messages[count] );
    struct networking_message_pool *pool;

    if (!(pool = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, size )))
    {
        ERR( "Failed to allocate memory for networking messages\n" );
        return FALSE;
    }
    pool->ref = count;

    while (count--) messages[count] = networking_message_init_144( &pool->messages[count], pool );
    return TRUE;
}

static BOOL networking_message_pool_receive_144( int32_t capacity, int32_t count, w_SteamNetworkingMessage_t_144 **messages )
{
    struct networking_message_pool *pool = CONTAINING_RECORD( messages[0], struct networking_message, w_msg_144 )->pool;
    int32_t i;

    if (count < 0) count = 0;

    for (i = count; i < capacity; i++)
    {
        messages[i]->m_pfnRelease( messages[i] );
        messages[i] = NULL;
    }

    if (count > 0)
    {
        struct steamclient_networking_messages_receive_144_params params = {.count = count, .w_msgs = messages};
        if (!networking_message_pool_alloc_data( count, pool )) return FALSE;
        STEAMCLIENT_CALL( steamclient_networking_messages_receive_144, &params );
    }

    return TRUE;
}

static void W_CDECL w_SteamNetworkingMessage_t_147_FreeData( w_SteamNetworkingMessage_t_147 *msg )
{
    struct networking_message *message = CONTAINING_RECORD( msg, struct networking_message, w_msg_147 );

    if (msg->m_pData) SecureZeroMemory( msg->m_pData, msg->m_cbSize );
    if (!message->pool) HeapFree( GetProcessHeap(), 0, msg->m_pData );
}

static void W_CDECL w_SteamNetworkingMessage_t_147_Release( w_SteamNetworkingMessage_t_147 *msg )
{
    struct networking_message *message = CONTAINING_RECORD( msg, struct networking_message, w_msg_147 );

    if (msg->m_pfnFreeData) msg->m_pfnFreeData( msg );
    SecureZeroMemory( msg, sizeof(*msg) );

    if (message->pool) networking_message_pool_release( message->pool );
    else
    {
        struct steamclient_networking_message_release_147_params params = {.w_msg = msg};
        STEAMCLIENT_CALL( steamclient_networking_message_release_147, &params );
        HeapFree( GetProcessHeap(), 0, message );
    }
}

static w_SteamNetworkingMessage_t_147 *networking_message_init_147( struct networking_message *message,
                                                                    struct networking_message_pool *pool )
{
    message->pool = pool;
    message->p_data = &message->w_msg_147.m_pData;
    message->p_size = (uint32_t *)&message->w_msg_147.m_cbSize;
    message->w_msg_147.m_pfnFreeData = w_SteamNetworkingMessage_t_147_FreeData;
    message->w_msg_147.m_pfnRelease = w_SteamNetworkingMessage_t_147_Release;
    return &message->w_msg_147;
}

static BOOL networking_message_pool_create_147( uint32_t count, w_SteamNetworkingMessage_t_147 **messages )
{
    uint32_t size = offsetof( struct networking_message_pool, messages[count] );
    struct networking_message_pool *pool;

    if (!(pool = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, size )))
    {
        ERR( "Failed to allocate memory for networking messages\n" );
        return FALSE;
    }
    pool->ref = count;

    while (count--) messages[count] = networking_message_init_147( &pool->messages[count], pool );
    return TRUE;
}

static BOOL networking_message_pool_receive_147( uint32_t capacity, int32_t count, w_SteamNetworkingMessage_t_147 **messages )
{
    struct networking_message_pool *pool = CONTAINING_RECORD( messages[0], struct networking_message, w_msg_147 )->pool;
    uint32_t i;

    if (count < 0) count = 0;

    for (i = count; i < capacity; i++)
    {
        messages[i]->m_pfnRelease( messages[i] );
        messages[i] = NULL;
    }

    if (count)
    {
        struct steamclient_networking_messages_receive_147_params params = {.count = count, .w_msgs = messages};
        if (!networking_message_pool_alloc_data( count, pool )) return FALSE;
        STEAMCLIENT_CALL( steamclient_networking_messages_receive_147, &params );
    }

    return TRUE;
}

static void W_CDECL w_SteamNetworkingMessage_t_153a_FreeData( w_SteamNetworkingMessage_t_153a *msg )
{
    struct networking_message *message = CONTAINING_RECORD( msg, struct networking_message, w_msg_153a );

    if (msg->m_pData) SecureZeroMemory( msg->m_pData, msg->m_cbSize );
    if (!message->pool) HeapFree( GetProcessHeap(), 0, msg->m_pData );
}

static void W_CDECL w_SteamNetworkingMessage_t_153a_Release( w_SteamNetworkingMessage_t_153a *msg )
{
    struct networking_message *message = CONTAINING_RECORD( msg, struct networking_message, w_msg_153a );

    if (msg->m_pfnFreeData) msg->m_pfnFreeData( msg );
    SecureZeroMemory( msg, sizeof(*msg) );

    if (message->pool) networking_message_pool_release( message->pool );
    else
    {
        struct steamclient_networking_message_release_153a_params params = {.w_msg = msg};
        STEAMCLIENT_CALL( steamclient_networking_message_release_153a, &params );
        HeapFree( GetProcessHeap(), 0, message );
    }
}

static w_SteamNetworkingMessage_t_153a *networking_message_init_153a( struct networking_message *message,
                                                                      struct networking_message_pool *pool )
{
    message->pool = pool;
    message->p_data = &message->w_msg_153a.m_pData;
    message->p_size = (uint32_t *)&message->w_msg_153a.m_cbSize;
    message->w_msg_153a.m_pfnFreeData = w_SteamNetworkingMessage_t_153a_FreeData;
    message->w_msg_153a.m_pfnRelease = w_SteamNetworkingMessage_t_153a_Release;
    return &message->w_msg_153a;
}

static BOOL networking_message_pool_create_153a( uint32_t count, w_SteamNetworkingMessage_t_153a **messages )
{
    uint32_t size = offsetof( struct networking_message_pool, messages[count] );
    struct networking_message_pool *pool;

    if (!(pool = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, size )))
    {
        ERR( "Failed to allocate memory for networking messages\n" );
        return FALSE;
    }
    pool->ref = count;

    while (count--) messages[count] = networking_message_init_153a( &pool->messages[count], pool );
    return TRUE;
}

static BOOL networking_message_pool_receive_153a( uint32_t capacity, int32_t count, w_SteamNetworkingMessage_t_153a **messages )
{
    struct networking_message_pool *pool = CONTAINING_RECORD( messages[0], struct networking_message, w_msg_153a )->pool;
    uint32_t i;

    if (count < 0) count = 0;

    for (i = count; i < capacity; i++)
    {
        messages[i]->m_pfnRelease( messages[i] );
        messages[i] = NULL;
    }

    if (count)
    {
        struct steamclient_networking_messages_receive_153a_params params = {.count = count, .w_msgs = messages};
        if (!networking_message_pool_alloc_data( count, pool )) return FALSE;
        STEAMCLIENT_CALL( steamclient_networking_messages_receive_153a, &params );
    }

    return TRUE;
}

/* ISteamNetworkingSockets_SteamNetworkingSockets002 */

int32_t __thiscall winISteamNetworkingSockets_SteamNetworkingSockets002_ReceiveMessagesOnConnection( struct w_iface *_this,
                                                                                                     uint32_t hConn, w_SteamNetworkingMessage_t_144 **ppOutMessages,
                                                                                                     int32_t nMaxMessages )
{
    struct ISteamNetworkingSockets_SteamNetworkingSockets002_ReceiveMessagesOnConnection_params params =
    {
        .u_iface = _this->u_iface,
        .hConn = hConn,
        .ppOutMessages = ppOutMessages,
        .nMaxMessages = nMaxMessages,
    };

    TRACE( "%p\n", _this );

    if (!networking_message_pool_create_144( nMaxMessages, params.ppOutMessages )) return 0;
    STEAMCLIENT_CALL( ISteamNetworkingSockets_SteamNetworkingSockets002_ReceiveMessagesOnConnection, &params );
    if (!networking_message_pool_receive_144( nMaxMessages, params._ret, params.ppOutMessages )) return 0;

    return params._ret;
}

int32_t __thiscall winISteamNetworkingSockets_SteamNetworkingSockets002_ReceiveMessagesOnListenSocket( struct w_iface *_this, uint32_t hSocket,
                                                                                                       w_SteamNetworkingMessage_t_144 **ppOutMessages, int32_t nMaxMessages )
{
    struct ISteamNetworkingSockets_SteamNetworkingSockets002_ReceiveMessagesOnListenSocket_params params =
    {
        .u_iface = _this->u_iface,
        .hSocket = hSocket,
        .ppOutMessages = ppOutMessages,
        .nMaxMessages = nMaxMessages,
    };

    TRACE( "%p\n", _this );

    if (!networking_message_pool_create_144( nMaxMessages, params.ppOutMessages )) return 0;
    STEAMCLIENT_CALL( ISteamNetworkingSockets_SteamNetworkingSockets002_ReceiveMessagesOnListenSocket, &params );
    if (!networking_message_pool_receive_144( nMaxMessages, params._ret, params.ppOutMessages )) return 0;

    return params._ret;
}

/* ISteamNetworkingSockets_SteamNetworkingSockets004 */

int32_t __thiscall winISteamNetworkingSockets_SteamNetworkingSockets004_ReceiveMessagesOnConnection( struct w_iface *_this,
                                                                                                     uint32_t hConn, w_SteamNetworkingMessage_t_144 **ppOutMessages,
                                                                                                     int32_t nMaxMessages )
{
    struct ISteamNetworkingSockets_SteamNetworkingSockets004_ReceiveMessagesOnConnection_params params =
    {
        .u_iface = _this->u_iface,
        .hConn = hConn,
        .ppOutMessages = ppOutMessages,
        .nMaxMessages = nMaxMessages,
    };

    TRACE( "%p\n", _this );

    if (!networking_message_pool_create_144( nMaxMessages, params.ppOutMessages )) return 0;
    STEAMCLIENT_CALL( ISteamNetworkingSockets_SteamNetworkingSockets004_ReceiveMessagesOnConnection, &params );
    if (!networking_message_pool_receive_144( nMaxMessages, params._ret, params.ppOutMessages )) return 0;

    return params._ret;
}

int32_t __thiscall winISteamNetworkingSockets_SteamNetworkingSockets004_ReceiveMessagesOnListenSocket( struct w_iface *_this, uint32_t hSocket,
                                                                                                       w_SteamNetworkingMessage_t_144 **ppOutMessages, int32_t nMaxMessages )
{
    struct ISteamNetworkingSockets_SteamNetworkingSockets004_ReceiveMessagesOnListenSocket_params params =
    {
        .u_iface = _this->u_iface,
        .hSocket = hSocket,
        .ppOutMessages = ppOutMessages,
        .nMaxMessages = nMaxMessages,
    };

    TRACE( "%p\n", _this );

    if (!networking_message_pool_create_144( nMaxMessages, params.ppOutMessages )) return 0;
    STEAMCLIENT_CALL( ISteamNetworkingSockets_SteamNetworkingSockets004_ReceiveMessagesOnListenSocket, &params );
    if (!networking_message_pool_receive_144( nMaxMessages, params._ret, params.ppOutMessages )) return 0;

    return params._ret;
}

/* ISteamNetworkingSockets_SteamNetworkingSockets006 */

int32_t __thiscall winISteamNetworkingSockets_SteamNetworkingSockets006_ReceiveMessagesOnConnection( struct w_iface *_this,
                                                                                                     uint32_t hConn, w_SteamNetworkingMessage_t_147 **ppOutMessages,
                                                                                                     int32_t nMaxMessages )
{
    struct ISteamNetworkingSockets_SteamNetworkingSockets006_ReceiveMessagesOnConnection_params params =
    {
        .u_iface = _this->u_iface,
        .hConn = hConn,
        .ppOutMessages = ppOutMessages,
        .nMaxMessages = nMaxMessages,
    };

    TRACE( "%p\n", _this );

    if (!networking_message_pool_create_147( nMaxMessages, params.ppOutMessages )) return 0;
    STEAMCLIENT_CALL( ISteamNetworkingSockets_SteamNetworkingSockets006_ReceiveMessagesOnConnection, &params );
    if (!networking_message_pool_receive_147( nMaxMessages, params._ret, params.ppOutMessages )) return 0;

    return params._ret;
}

int32_t __thiscall winISteamNetworkingSockets_SteamNetworkingSockets006_ReceiveMessagesOnListenSocket( struct w_iface *_this, uint32_t hSocket,
                                                                                                       w_SteamNetworkingMessage_t_147 **ppOutMessages, int32_t nMaxMessages )
{
    struct ISteamNetworkingSockets_SteamNetworkingSockets006_ReceiveMessagesOnListenSocket_params params =
    {
        .u_iface = _this->u_iface,
        .hSocket = hSocket,
        .ppOutMessages = ppOutMessages,
        .nMaxMessages = nMaxMessages,
    };

    TRACE( "%p\n", _this );

    if (!networking_message_pool_create_147( nMaxMessages, params.ppOutMessages )) return 0;
    STEAMCLIENT_CALL( ISteamNetworkingSockets_SteamNetworkingSockets006_ReceiveMessagesOnListenSocket, &params );
    if (!networking_message_pool_receive_147( nMaxMessages, params._ret, params.ppOutMessages )) return 0;

    return params._ret;
}

void __thiscall winISteamNetworkingSockets_SteamNetworkingSockets006_SendMessages(struct w_iface *_this, int32_t nMessages, w_SteamNetworkingMessage_t_147 **pMessages, int64_t *pOutMessageNumberOrResult)
{
    struct ISteamNetworkingSockets_SteamNetworkingSockets006_SendMessages_params params =
    {
        .u_iface = _this->u_iface,
        .nMessages = nMessages,
        .pMessages = pMessages,
        .pOutMessageNumberOrResult = pOutMessageNumberOrResult,
    };
    int64_t i;

    TRACE("%p\n", _this);

    STEAMCLIENT_CALL( ISteamNetworkingSockets_SteamNetworkingSockets006_SendMessages, &params );
    for (i = 0; i < nMessages; i++) pMessages[i]->m_pfnRelease( pMessages[i] );
}

/* ISteamNetworkingSockets_SteamNetworkingSockets008 */

int32_t __thiscall winISteamNetworkingSockets_SteamNetworkingSockets008_ReceiveMessagesOnConnection( struct w_iface *_this,
                                                                                                     uint32_t hConn, w_SteamNetworkingMessage_t_147 **ppOutMessages,
                                                                                                     int32_t nMaxMessages )
{
    struct ISteamNetworkingSockets_SteamNetworkingSockets008_ReceiveMessagesOnConnection_params params =
    {
        .u_iface = _this->u_iface,
        .hConn = hConn,
        .ppOutMessages = ppOutMessages,
        .nMaxMessages = nMaxMessages,
    };

    TRACE( "%p\n", _this );

    if (!networking_message_pool_create_147( nMaxMessages, params.ppOutMessages )) return 0;
    STEAMCLIENT_CALL( ISteamNetworkingSockets_SteamNetworkingSockets008_ReceiveMessagesOnConnection, &params );
    if (!networking_message_pool_receive_147( nMaxMessages, params._ret, params.ppOutMessages )) return 0;

    return params._ret;
}

int32_t __thiscall winISteamNetworkingSockets_SteamNetworkingSockets008_ReceiveMessagesOnPollGroup( struct w_iface *_this, uint32_t hPollGroup,
                                                                                                    w_SteamNetworkingMessage_t_147 **ppOutMessages,
                                                                                                    int32_t nMaxMessages )
{
    struct ISteamNetworkingSockets_SteamNetworkingSockets008_ReceiveMessagesOnPollGroup_params params =
    {
        .u_iface = _this->u_iface,
        .hPollGroup = hPollGroup,
        .ppOutMessages = ppOutMessages,
        .nMaxMessages = nMaxMessages,
    };

    TRACE( "%p\n", _this );

    if (!networking_message_pool_create_147( nMaxMessages, params.ppOutMessages )) return 0;
    STEAMCLIENT_CALL( ISteamNetworkingSockets_SteamNetworkingSockets008_ReceiveMessagesOnPollGroup, &params );
    if (!networking_message_pool_receive_147( nMaxMessages, params._ret, params.ppOutMessages )) return 0;

    return params._ret;
}

void __thiscall winISteamNetworkingSockets_SteamNetworkingSockets008_SendMessages(struct w_iface *_this, int32_t nMessages, w_SteamNetworkingMessage_t_147 *const *pMessages, int64_t *pOutMessageNumberOrResult)
{
    struct ISteamNetworkingSockets_SteamNetworkingSockets008_SendMessages_params params =
    {
        .u_iface = _this->u_iface,
        .nMessages = nMessages,
        .pMessages = pMessages,
        .pOutMessageNumberOrResult = pOutMessageNumberOrResult,
    };
    int64_t i;

    TRACE("%p\n", _this);

    STEAMCLIENT_CALL( ISteamNetworkingSockets_SteamNetworkingSockets008_SendMessages, &params );
    for (i = 0; i < nMessages; i++) pMessages[i]->m_pfnRelease( pMessages[i] );
}

/* ISteamNetworkingSockets_SteamNetworkingSockets009 */

void __thiscall winISteamNetworkingSockets_SteamNetworkingSockets009_RunCallbacks(struct w_iface *_this)
{
    struct ISteamNetworkingSockets_SteamNetworkingSockets009_RunCallbacks_params params =
    {
        .u_iface = _this->u_iface,
    };
    TRACE("%p\n", _this);
    STEAMCLIENT_CALL( ISteamNetworkingSockets_SteamNetworkingSockets009_RunCallbacks, &params );
    execute_pending_callbacks();
}

int32_t __thiscall winISteamNetworkingSockets_SteamNetworkingSockets009_ReceiveMessagesOnConnection( struct w_iface *_this,
                                                                                                     uint32_t hConn, w_SteamNetworkingMessage_t_147 **ppOutMessages,
                                                                                                     int32_t nMaxMessages )
{
    struct ISteamNetworkingSockets_SteamNetworkingSockets009_ReceiveMessagesOnConnection_params params =
    {
        .u_iface = _this->u_iface,
        .hConn = hConn,
        .ppOutMessages = ppOutMessages,
        .nMaxMessages = nMaxMessages,
    };

    TRACE( "%p\n", _this );

    if (!networking_message_pool_create_147( nMaxMessages, params.ppOutMessages )) return 0;
    STEAMCLIENT_CALL( ISteamNetworkingSockets_SteamNetworkingSockets009_ReceiveMessagesOnConnection, &params );
    if (!networking_message_pool_receive_147( nMaxMessages, params._ret, params.ppOutMessages )) return 0;

    return params._ret;
}

int32_t __thiscall winISteamNetworkingSockets_SteamNetworkingSockets009_ReceiveMessagesOnPollGroup( struct w_iface *_this, uint32_t hPollGroup,
                                                                                                    w_SteamNetworkingMessage_t_147 **ppOutMessages,
                                                                                                    int32_t nMaxMessages )
{
    struct ISteamNetworkingSockets_SteamNetworkingSockets009_ReceiveMessagesOnPollGroup_params params =
    {
        .u_iface = _this->u_iface,
        .hPollGroup = hPollGroup,
        .ppOutMessages = ppOutMessages,
        .nMaxMessages = nMaxMessages,
    };

    TRACE( "%p\n", _this );

    if (!networking_message_pool_create_147( nMaxMessages, params.ppOutMessages )) return 0;
    STEAMCLIENT_CALL( ISteamNetworkingSockets_SteamNetworkingSockets009_ReceiveMessagesOnPollGroup, &params );
    if (!networking_message_pool_receive_147( nMaxMessages, params._ret, params.ppOutMessages )) return 0;

    return params._ret;
}

void __thiscall winISteamNetworkingSockets_SteamNetworkingSockets009_SendMessages(struct w_iface *_this, int32_t nMessages, w_SteamNetworkingMessage_t_147 *const *pMessages, int64_t *pOutMessageNumberOrResult)
{
    struct ISteamNetworkingSockets_SteamNetworkingSockets009_SendMessages_params params =
    {
        .u_iface = _this->u_iface,
        .nMessages = nMessages,
        .pMessages = pMessages,
        .pOutMessageNumberOrResult = pOutMessageNumberOrResult,
    };
    int64_t i;

    TRACE("%p\n", _this);

    STEAMCLIENT_CALL( ISteamNetworkingSockets_SteamNetworkingSockets009_SendMessages, &params );
    for (i = 0; i < nMessages; i++) pMessages[i]->m_pfnRelease( pMessages[i] );
}

/* ISteamNetworkingUtils_SteamNetworkingUtils003 */

w_SteamNetworkingMessage_t_147 *__thiscall winISteamNetworkingUtils_SteamNetworkingUtils003_AllocateMessage( struct w_iface *_this, int32_t cbAllocateBuffer )
{
    struct ISteamNetworkingUtils_SteamNetworkingUtils003_AllocateMessage_params params =
    {
        .u_iface = _this->u_iface,
        .cbAllocateBuffer = cbAllocateBuffer,
    };
    struct networking_message *message;

    TRACE( "%p\n", _this );

    if (!(message = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*message) ))) return NULL;
    if ((message->w_msg_147.m_cbSize = cbAllocateBuffer) &&
        !(message->w_msg_147.m_pData = HeapAlloc( GetProcessHeap(), 0, cbAllocateBuffer )))
    {
        HeapFree( GetProcessHeap(), 0, message );
        return NULL;
    }
    message->w_msg_147.m_pfnFreeData = w_SteamNetworkingMessage_t_147_FreeData;
    message->w_msg_147.m_pfnRelease = w_SteamNetworkingMessage_t_147_Release;
    params._ret = &message->w_msg_147;

    STEAMCLIENT_CALL( ISteamNetworkingUtils_SteamNetworkingUtils003_AllocateMessage, &params );

    return params._ret;
}

/* ISteamNetworkingFakeUDPPort_SteamNetworkingFakeUDPPort001 */

void __thiscall winISteamNetworkingFakeUDPPort_SteamNetworkingFakeUDPPort001_DestroyFakeUDPPort( struct w_iface *_this )
{
    struct ISteamNetworkingFakeUDPPort_SteamNetworkingFakeUDPPort001_DestroyFakeUDPPort_params params = {.u_iface = _this->u_iface};
    TRACE( "%p\n", _this );
    STEAMCLIENT_CALL( ISteamNetworkingFakeUDPPort_SteamNetworkingFakeUDPPort001_DestroyFakeUDPPort, &params );
    HeapFree( GetProcessHeap(), 0, _this );
}

int32_t __thiscall winISteamNetworkingFakeUDPPort_SteamNetworkingFakeUDPPort001_ReceiveMessages( struct w_iface *_this,
                                                                                                 w_SteamNetworkingMessage_t_153a **ppOutMessages,
                                                                                                 int32_t nMaxMessages )
{
    struct ISteamNetworkingFakeUDPPort_SteamNetworkingFakeUDPPort001_ReceiveMessages_params params =
    {
        .u_iface = _this->u_iface,
        .ppOutMessages = ppOutMessages,
        .nMaxMessages = nMaxMessages,
    };

    TRACE( "%p\n", _this );

    if (!networking_message_pool_create_153a( nMaxMessages, params.ppOutMessages )) return 0;
    STEAMCLIENT_CALL( ISteamNetworkingFakeUDPPort_SteamNetworkingFakeUDPPort001_ReceiveMessages, &params );
    if (!networking_message_pool_receive_153a( nMaxMessages, params._ret, params.ppOutMessages )) return 0;

    return params._ret;
}

/* ISteamNetworkingMessages_SteamNetworkingMessages002 */

int32_t __thiscall winISteamNetworkingMessages_SteamNetworkingMessages002_ReceiveMessagesOnChannel( struct w_iface *_this, int32_t nLocalChannel,
                                                                                                    w_SteamNetworkingMessage_t_153a **ppOutMessages,
                                                                                                    int32_t nMaxMessages )
{
    struct ISteamNetworkingMessages_SteamNetworkingMessages002_ReceiveMessagesOnChannel_params params =
    {
        .u_iface = _this->u_iface,
        .nLocalChannel = nLocalChannel,
        .ppOutMessages = ppOutMessages,
        .nMaxMessages = nMaxMessages,
    };

    TRACE( "%p\n", _this );

    if (!networking_message_pool_create_153a( nMaxMessages, params.ppOutMessages )) return 0;
    STEAMCLIENT_CALL( ISteamNetworkingMessages_SteamNetworkingMessages002_ReceiveMessagesOnChannel, &params );
    if (!networking_message_pool_receive_153a( nMaxMessages, params._ret, params.ppOutMessages )) return 0;

    return params._ret;
}

/* ISteamNetworkingSockets_SteamNetworkingSockets012 */

void __thiscall winISteamNetworkingSockets_SteamNetworkingSockets012_RunCallbacks(struct w_iface *_this)
{
    struct ISteamNetworkingSockets_SteamNetworkingSockets012_RunCallbacks_params params =
    {
        .u_iface = _this->u_iface,
    };
    TRACE("%p\n", _this);
    STEAMCLIENT_CALL( ISteamNetworkingSockets_SteamNetworkingSockets012_RunCallbacks, &params );
    execute_pending_callbacks();
    TRACE("done.\n");
}

int32_t __thiscall winISteamNetworkingSockets_SteamNetworkingSockets012_ReceiveMessagesOnConnection( struct w_iface *_this,
                                                                                                     uint32_t hConn, w_SteamNetworkingMessage_t_153a **ppOutMessages,
                                                                                                     int32_t nMaxMessages )
{
    struct ISteamNetworkingSockets_SteamNetworkingSockets012_ReceiveMessagesOnConnection_params params =
    {
        .u_iface = _this->u_iface,
        .hConn = hConn,
        .ppOutMessages = ppOutMessages,
        .nMaxMessages = nMaxMessages,
    };

    TRACE( "%p\n", _this );

    if (!networking_message_pool_create_153a( nMaxMessages, params.ppOutMessages )) return 0;
    STEAMCLIENT_CALL( ISteamNetworkingSockets_SteamNetworkingSockets012_ReceiveMessagesOnConnection, &params );
    if (!networking_message_pool_receive_153a( nMaxMessages, params._ret, params.ppOutMessages )) return 0;

    return params._ret;
}

int32_t __thiscall winISteamNetworkingSockets_SteamNetworkingSockets012_ReceiveMessagesOnPollGroup( struct w_iface *_this, uint32_t hPollGroup,
                                                                                                    w_SteamNetworkingMessage_t_153a **ppOutMessages,
                                                                                                    int32_t nMaxMessages )
{
    struct ISteamNetworkingSockets_SteamNetworkingSockets012_ReceiveMessagesOnPollGroup_params params =
    {
        .u_iface = _this->u_iface,
        .hPollGroup = hPollGroup,
        .ppOutMessages = ppOutMessages,
        .nMaxMessages = nMaxMessages,
    };

    TRACE( "%p\n", _this );

    if (!networking_message_pool_create_153a( nMaxMessages, params.ppOutMessages )) return 0;
    STEAMCLIENT_CALL( ISteamNetworkingSockets_SteamNetworkingSockets012_ReceiveMessagesOnPollGroup, &params );
    if (!networking_message_pool_receive_153a( nMaxMessages, params._ret, params.ppOutMessages )) return 0;

    return params._ret;
}

/* ISteamNetworkingUtils_SteamNetworkingUtils004 */

void __thiscall winISteamNetworkingSockets_SteamNetworkingSockets012_SendMessages(struct w_iface *_this, int32_t nMessages, w_SteamNetworkingMessage_t_153a *const *pMessages, int64_t *pOutMessageNumberOrResult)
{
    struct ISteamNetworkingSockets_SteamNetworkingSockets012_SendMessages_params params =
    {
        .u_iface = _this->u_iface,
        .nMessages = nMessages,
        .pMessages = pMessages,
        .pOutMessageNumberOrResult = pOutMessageNumberOrResult,
    };
    int64_t i;

    TRACE("%p\n", _this);

    STEAMCLIENT_CALL( ISteamNetworkingSockets_SteamNetworkingSockets012_SendMessages, &params );
    for (i = 0; i < nMessages; i++) pMessages[i]->m_pfnRelease( pMessages[i] );
}

w_SteamNetworkingMessage_t_153a *__thiscall winISteamNetworkingUtils_SteamNetworkingUtils004_AllocateMessage( struct w_iface *_this, int32_t cbAllocateBuffer )
{
    struct ISteamNetworkingUtils_SteamNetworkingUtils004_AllocateMessage_params params =
    {
        .u_iface = _this->u_iface,
        .cbAllocateBuffer = cbAllocateBuffer,
    };
    struct networking_message *message;

    TRACE( "%p\n", _this );

    if (!(message = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*message) ))) return NULL;
    if ((message->w_msg_153a.m_cbSize = cbAllocateBuffer) &&
        !(message->w_msg_153a.m_pData = HeapAlloc( GetProcessHeap(), 0, cbAllocateBuffer )))
    {
        HeapFree( GetProcessHeap(), 0, message );
        return NULL;
    }
    message->w_msg_153a.m_pfnFreeData = w_SteamNetworkingMessage_t_153a_FreeData;
    message->w_msg_153a.m_pfnRelease = w_SteamNetworkingMessage_t_153a_Release;
    params._ret = &message->w_msg_153a;

    STEAMCLIENT_CALL( ISteamNetworkingUtils_SteamNetworkingUtils004_AllocateMessage, &params );
    return params._ret;
}

/* === Legacy ISteamNetworking005 P2P -> ISteamNetworkingSockets012 ConnectP2P bridge ===
 * Valve's Android libsteamclient stubs the legacy P2P socket transport
 * ("CreateSNetSocketForP2P not implemented for Android,iOS,tvOS"), so legacy
 * SendP2PPacket queues then times out. We reimplement the six SteamNetworking005
 * P2P methods over modern ISteamNetworkingSockets, exactly as desktop Steam does
 * internally: desktop legacy ISteamNetworking P2P and ConnectP2P share the same
 * SNS rendezvous wire message (CMsgSteamDatagramP2PSessionRequest), and the
 * legacy / plain-P2P remote virtual port is -1. (k_nVirtualPort_Messages =
 * 0x7fffffff is a DIFFERENT subsystem -- a prior Messages002 bridge connected on
 * 0x7fffffff and a desktop legacy host had nothing listening there, so it stayed
 * silent and timed out.) One SNS connection per remote peer; legacy channels are
 * not separately framed over SNS (AoE2 uses channel 0). The Sockets012 wrapper is
 * captured via p2p_bridge_set_sockets() in winISteamClient_*_GetISteamNetworking.
 * (DEFINE_THISCALL_WRAPPER + VTABLE_ADD_FUNC for these remain in winISteamNetworking.c.) */
extern uint32_t __thiscall winISteamNetworkingSockets_SteamNetworkingSockets012_ConnectP2P(struct w_iface *, const SteamNetworkingIdentity_144 *, int32_t, int32_t, const void *);
extern uint32_t __thiscall winISteamNetworkingSockets_SteamNetworkingSockets012_SendMessageToConnection(struct w_iface *, uint32_t, const void *, uint32_t, int32_t, int64_t *);
extern int8_t   __thiscall winISteamNetworkingSockets_SteamNetworkingSockets012_GetConnectionInfo(struct w_iface *, uint32_t, SteamNetConnectionInfo_t_153a *);
extern int8_t   __thiscall winISteamNetworkingSockets_SteamNetworkingSockets012_CloseConnection(struct w_iface *, uint32_t, int32_t, const char *, int8_t);
/* winISteamNetworkingSockets_SteamNetworkingSockets012_ReceiveMessagesOnConnection is defined above. */

#define P2P_BR_PEERS 32    /* concurrent remote peers */
#define P2P_BR_Q     512   /* global receive FIFO depth */
static struct w_iface *g_p2p_sockets;
static struct { uint64_t steam; uint32_t hConn; int8_t got_data; } g_p2p_peer[P2P_BR_PEERS];
static int g_p2p_npeers;
static w_SteamNetworkingMessage_t_153a *g_p2p_rx[P2P_BR_Q];
static int g_p2p_rxh, g_p2p_rxt;
static CRITICAL_SECTION g_p2p_cs;
static BOOL g_p2p_cs_ok;

void p2p_bridge_set_sockets(struct w_iface *s)
{
    if (!g_p2p_cs_ok) { InitializeCriticalSection(&g_p2p_cs); g_p2p_cs_ok = TRUE; }
    g_p2p_sockets = s;
    TRACE("p2p_bridge: NetworkingSockets012 wrapper = %p\n", s);
}

static void p2p_make_id(SteamNetworkingIdentity_144 *id, uint64_t s)
{
    memset(id, 0, sizeof(*id));
    id->m_eType = 16;   /* k_ESteamNetworkingIdentityType_SteamID */
    id->m_cbSize = 8;
    memcpy(&id->data, &s, sizeof(s));
}

/* Find (and optionally open) the per-peer SNS connection. Caller holds g_p2p_cs. */
static uint32_t p2p_conn_for(uint64_t steam, int create)
{
    SteamNetworkingIdentity_144 id;
    uint32_t h;
    int i;
    for (i = 0; i < g_p2p_npeers; i++)
        if (g_p2p_peer[i].steam == steam) return g_p2p_peer[i].hConn;
    if (!create || g_p2p_npeers >= P2P_BR_PEERS) return 0;
    p2p_make_id(&id, steam);
    /* nRemoteVirtualPort = -1 (legacy/plain P2P); no options. */
    h = winISteamNetworkingSockets_SteamNetworkingSockets012_ConnectP2P(g_p2p_sockets, &id, -1, 0, NULL);
    if (!h) { ERR("p2p_bridge: ConnectP2P to %llx failed\n", (unsigned long long)steam); return 0; }
    g_p2p_peer[g_p2p_npeers].steam = steam;
    g_p2p_peer[g_p2p_npeers].hConn = h;
    g_p2p_peer[g_p2p_npeers].got_data = 0;
    g_p2p_npeers++;
    TRACE("p2p_bridge: ConnectP2P(vport=-1) to %llx -> hConn %u\n", (unsigned long long)steam, h);
    return h;
}

/* Drain every peer connection into the global FIFO. Caller holds g_p2p_cs. */
static void p2p_rx_refill(void)
{
    w_SteamNetworkingMessage_t_153a *tmp[64];
    int i, n, k;
    for (i = 0; i < g_p2p_npeers; i++)
    {
        if (!g_p2p_peer[i].hConn) continue;
        n = winISteamNetworkingSockets_SteamNetworkingSockets012_ReceiveMessagesOnConnection(g_p2p_sockets, g_p2p_peer[i].hConn, tmp, 64);
        if (n > 0) g_p2p_peer[i].got_data = 1;
        for (k = 0; k < n; k++)
        {
            int nt = (g_p2p_rxt + 1) % P2P_BR_Q;
            if (nt == g_p2p_rxh) { tmp[k]->m_pfnRelease(tmp[k]); continue; }  /* full */
            g_p2p_rx[g_p2p_rxt] = tmp[k];
            g_p2p_rxt = nt;
        }
    }
}

int8_t __thiscall winISteamNetworking_SteamNetworking005_SendP2PPacket(struct w_iface *_this, CSteamID steamIDRemote, const void *pubData, uint32_t cubData, uint32_t eP2PSendType, int32_t nChannel)
{
    uint32_t h, r;
    TRACE("%p\n", _this);
    if (!g_p2p_sockets) return 0;
    EnterCriticalSection(&g_p2p_cs);
    h = p2p_conn_for(*(uint64_t *)&steamIDRemote, 1);
    LeaveCriticalSection(&g_p2p_cs);
    if (!h) return 0;
    /* EP2PSend: Reliable(2)/ReliableWithBuffering(3) -> Reliable(8); else Unreliable(0). */
    r = winISteamNetworkingSockets_SteamNetworkingSockets012_SendMessageToConnection(
            g_p2p_sockets, h, pubData, cubData, (eP2PSendType == 2 || eP2PSendType == 3) ? 8 : 0, NULL);
    return r == 1;   /* k_EResultOK */
}

int8_t __thiscall winISteamNetworking_SteamNetworking005_IsP2PPacketAvailable(struct w_iface *_this, uint32_t *pcubMsgSize, int32_t nChannel)
{
    int8_t r = 0;
    TRACE("%p\n", _this);
    if (!g_p2p_sockets) return 0;
    EnterCriticalSection(&g_p2p_cs);
    p2p_rx_refill();
    if (g_p2p_rxh != g_p2p_rxt)
    {
        if (pcubMsgSize) *pcubMsgSize = g_p2p_rx[g_p2p_rxh]->m_cbSize;
        r = 1;
    }
    LeaveCriticalSection(&g_p2p_cs);
    return r;
}

int8_t __thiscall winISteamNetworking_SteamNetworking005_ReadP2PPacket(struct w_iface *_this, void *pubDest, uint32_t cubDest, uint32_t *pcubMsgSize, CSteamID *psteamIDRemote, int32_t nChannel)
{
    int8_t r = 0;
    TRACE("%p\n", _this);
    if (!g_p2p_sockets) return 0;
    EnterCriticalSection(&g_p2p_cs);
    p2p_rx_refill();
    if (g_p2p_rxh != g_p2p_rxt)
    {
        w_SteamNetworkingMessage_t_153a *m = g_p2p_rx[g_p2p_rxh];
        uint32_t sz = m->m_cbSize;
        g_p2p_rxh = (g_p2p_rxh + 1) % P2P_BR_Q;
        if (sz > cubDest) sz = cubDest;
        if (pubDest && m->m_pData) memcpy(pubDest, m->m_pData, sz);
        if (pcubMsgSize) *pcubMsgSize = sz;
        if (psteamIDRemote) memcpy(psteamIDRemote, &m->m_identityPeer.data, 8);   /* peer SteamID64 */
        m->m_pfnRelease(m);
        r = 1;
    }
    LeaveCriticalSection(&g_p2p_cs);
    return r;
}

int8_t __thiscall winISteamNetworking_SteamNetworking005_AcceptP2PSessionWithUser(struct w_iface *_this, CSteamID steamIDRemote)
{
    uint32_t h;
    TRACE("%p\n", _this);
    if (!g_p2p_sockets) return 0;
    EnterCriticalSection(&g_p2p_cs);
    h = p2p_conn_for(*(uint64_t *)&steamIDRemote, 1);   /* ensure a connection to the peer exists */
    LeaveCriticalSection(&g_p2p_cs);
    return h != 0;
}

int8_t __thiscall winISteamNetworking_SteamNetworking005_CloseP2PSessionWithUser(struct w_iface *_this, CSteamID steamIDRemote)
{
    uint64_t steam = *(uint64_t *)&steamIDRemote;
    int i;
    TRACE("%p\n", _this);
    if (!g_p2p_sockets) return 0;
    EnterCriticalSection(&g_p2p_cs);
    for (i = 0; i < g_p2p_npeers; i++)
    {
        if (g_p2p_peer[i].steam != steam) continue;
        if (g_p2p_peer[i].hConn)
            winISteamNetworkingSockets_SteamNetworkingSockets012_CloseConnection(g_p2p_sockets, g_p2p_peer[i].hConn, 0, NULL, 0);
        g_p2p_peer[i] = g_p2p_peer[--g_p2p_npeers];
        break;
    }
    LeaveCriticalSection(&g_p2p_cs);
    return 1;
}

int8_t __thiscall winISteamNetworking_SteamNetworking005_GetP2PSessionState(struct w_iface *_this, CSteamID steamIDRemote, P2PSessionState_t *pConnectionState)
{
    SteamNetConnectionInfo_t_153a info;
    uint32_t h;
    TRACE("%p\n", _this);
    if (!pConnectionState) return 0;
    memset(pConnectionState, 0, sizeof(*pConnectionState));
    if (!g_p2p_sockets) return 0;
    EnterCriticalSection(&g_p2p_cs);
    h = p2p_conn_for(*(uint64_t *)&steamIDRemote, 0);
    LeaveCriticalSection(&g_p2p_cs);
    if (!h) return 0;
    memset(&info, 0, sizeof(info));
    if (winISteamNetworkingSockets_SteamNetworkingSockets012_GetConnectionInfo(g_p2p_sockets, h, &info))
    {
        /* m_eState: Connecting=1, FindingRoute=2, Connected=3 */
        if (info.m_eState == 3)                         pConnectionState->m_bConnectionActive = 1;
        else if (info.m_eState == 1 || info.m_eState == 2) pConnectionState->m_bConnecting = 1;
    }
    else pConnectionState->m_bConnecting = 1;
    return 1;
}
