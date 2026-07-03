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

/* === Legacy ISteamNetworking005 P2P -> ISteamNetworkingMessages002 bridge ===
 * Valve's Android libsteamclient stubs the legacy P2P socket transport
 * ("CreateSNetSocketForP2P not implemented for Android, iOS, tvOS"), so the old
 * ISteamNetworking005 SendP2PPacket queues then times out -- never connects.
 * The modern ISteamNetworkingMessages002 (SNS-backed; relays come up fine) IS
 * implemented and maps ~1:1, so these six SteamNetworking005 P2P methods route
 * to it. The Messages002 wrapper is captured in winISteamClient_*_GetISteamNetworking
 * via p2p_bridge_set_messages(). Interop is sound: desktop Steam already runs
 * ISteamNetworking005 over SNS, so both ends speak SNS P2P.
 * (DEFINE_THISCALL_WRAPPER + VTABLE_ADD_FUNC for these remain in winISteamNetworking.c.) */
extern uint32_t __thiscall winISteamNetworkingMessages_SteamNetworkingMessages002_SendMessageToUser(struct w_iface *, const SteamNetworkingIdentity_144 *, const void *, uint32_t, int32_t, int32_t);
extern int8_t   __thiscall winISteamNetworkingMessages_SteamNetworkingMessages002_AcceptSessionWithUser(struct w_iface *, const SteamNetworkingIdentity_144 *);
extern int8_t   __thiscall winISteamNetworkingMessages_SteamNetworkingMessages002_CloseSessionWithUser(struct w_iface *, const SteamNetworkingIdentity_144 *);
extern uint32_t __thiscall winISteamNetworkingMessages_SteamNetworkingMessages002_GetSessionConnectionInfo(struct w_iface *, const SteamNetworkingIdentity_144 *, void *, void *);
extern int32_t  __thiscall winISteamNetworkingMessages_SteamNetworkingMessages002_ReceiveMessagesOnChannel(struct w_iface *, int32_t, w_SteamNetworkingMessage_t_153a **, int32_t);

#define P2P_BR_CH 8     /* channels buffered */
#define P2P_BR_Q  256   /* per-channel queue depth */
static struct w_iface *g_p2p_msgs;
static w_SteamNetworkingMessage_t_153a *g_p2p_q[P2P_BR_CH][P2P_BR_Q];
static int g_p2p_h[P2P_BR_CH], g_p2p_t[P2P_BR_CH];
static CRITICAL_SECTION g_p2p_cs;
static BOOL g_p2p_cs_ok;

void p2p_bridge_set_messages(struct w_iface *m)
{
    if (!g_p2p_cs_ok) { InitializeCriticalSection(&g_p2p_cs); g_p2p_cs_ok = TRUE; }
    g_p2p_msgs = m;
    TRACE("p2p_bridge: Messages002 wrapper = %p\n", m);
}

static void p2p_make_id(SteamNetworkingIdentity_144 *id, uint64_t s)
{
    memset(id, 0, sizeof(*id));
    id->m_eType = 16;   /* k_ESteamNetworkingIdentityType_SteamID */
    id->m_cbSize = 8;
    memcpy(&id->data, &s, sizeof(s));
}

/* Pull pending messages for channel `ch` into the FIFO. Caller holds g_p2p_cs. */
static void p2p_refill(int ch)
{
    w_SteamNetworkingMessage_t_153a *tmp[64];
    int n, i;
    if (g_p2p_h[ch] != g_p2p_t[ch]) return;   /* still draining */
    n = winISteamNetworkingMessages_SteamNetworkingMessages002_ReceiveMessagesOnChannel(g_p2p_msgs, ch, tmp, 64);
    for (i = 0; i < n; i++)
    {
        int nt = (g_p2p_t[ch] + 1) % P2P_BR_Q;
        if (nt == g_p2p_h[ch]) { tmp[i]->m_pfnRelease(tmp[i]); continue; }  /* full */
        g_p2p_q[ch][g_p2p_t[ch]] = tmp[i];
        g_p2p_t[ch] = nt;
    }
}

int8_t __thiscall winISteamNetworking_SteamNetworking005_SendP2PPacket(struct w_iface *_this, CSteamID steamIDRemote, const void *pubData, uint32_t cubData, uint32_t eP2PSendType, int32_t nChannel)
{
    SteamNetworkingIdentity_144 id;
    TRACE("%p\n", _this);
    if (!g_p2p_msgs) return 0;
    p2p_make_id(&id, *(uint64_t *)&steamIDRemote);
    /* EP2PSend: Reliable(2)/ReliableWithBuffering(3) -> Reliable(8); else Unreliable(0). */
    return winISteamNetworkingMessages_SteamNetworkingMessages002_SendMessageToUser(
               g_p2p_msgs, &id, pubData, cubData, (eP2PSendType == 2 || eP2PSendType == 3) ? 8 : 0, nChannel) == 1;
}

int8_t __thiscall winISteamNetworking_SteamNetworking005_IsP2PPacketAvailable(struct w_iface *_this, uint32_t *pcubMsgSize, int32_t nChannel)
{
    int8_t r = 0;
    TRACE("%p\n", _this);
    if (!g_p2p_msgs || nChannel < 0 || nChannel >= P2P_BR_CH) return 0;
    EnterCriticalSection(&g_p2p_cs);
    p2p_refill(nChannel);
    if (g_p2p_h[nChannel] != g_p2p_t[nChannel])
    {
        if (pcubMsgSize) *pcubMsgSize = g_p2p_q[nChannel][g_p2p_h[nChannel]]->m_cbSize;
        r = 1;
    }
    LeaveCriticalSection(&g_p2p_cs);
    return r;
}

int8_t __thiscall winISteamNetworking_SteamNetworking005_ReadP2PPacket(struct w_iface *_this, void *pubDest, uint32_t cubDest, uint32_t *pcubMsgSize, CSteamID *psteamIDRemote, int32_t nChannel)
{
    int8_t r = 0;
    TRACE("%p\n", _this);
    if (!g_p2p_msgs || nChannel < 0 || nChannel >= P2P_BR_CH) return 0;
    EnterCriticalSection(&g_p2p_cs);
    p2p_refill(nChannel);
    if (g_p2p_h[nChannel] != g_p2p_t[nChannel])
    {
        w_SteamNetworkingMessage_t_153a *m = g_p2p_q[nChannel][g_p2p_h[nChannel]];
        uint32_t sz = m->m_cbSize;
        g_p2p_h[nChannel] = (g_p2p_h[nChannel] + 1) % P2P_BR_Q;
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
    SteamNetworkingIdentity_144 id;
    TRACE("%p\n", _this);
    if (!g_p2p_msgs) return 0;
    p2p_make_id(&id, *(uint64_t *)&steamIDRemote);
    return winISteamNetworkingMessages_SteamNetworkingMessages002_AcceptSessionWithUser(g_p2p_msgs, &id);
}

int8_t __thiscall winISteamNetworking_SteamNetworking005_CloseP2PSessionWithUser(struct w_iface *_this, CSteamID steamIDRemote)
{
    SteamNetworkingIdentity_144 id;
    TRACE("%p\n", _this);
    if (!g_p2p_msgs) return 0;
    p2p_make_id(&id, *(uint64_t *)&steamIDRemote);
    return winISteamNetworkingMessages_SteamNetworkingMessages002_CloseSessionWithUser(g_p2p_msgs, &id);
}

int8_t __thiscall winISteamNetworking_SteamNetworking005_GetP2PSessionState(struct w_iface *_this, CSteamID steamIDRemote, P2PSessionState_t *pConnectionState)
{
    SteamNetworkingIdentity_144 id;
    int s;
    TRACE("%p\n", _this);
    if (!pConnectionState) return 0;
    memset(pConnectionState, 0, sizeof(*pConnectionState));
    if (!g_p2p_msgs) return 0;
    p2p_make_id(&id, *(uint64_t *)&steamIDRemote);
    s = winISteamNetworkingMessages_SteamNetworkingMessages002_GetSessionConnectionInfo(g_p2p_msgs, &id, NULL, NULL);
    if (s == 3)                 pConnectionState->m_bConnectionActive = 1;  /* Connected */
    else if (s == 1 || s == 2)  pConnectionState->m_bConnecting = 1;        /* Connecting / FindingRoute */
    return 1;
}
