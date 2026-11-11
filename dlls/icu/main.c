/*
 * ICU forwarding library initialization
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "windef.h"
#include "winbase.h"
#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(icu);

static HMODULE icuuc, icuin;

BOOL WINAPI DllMain( HINSTANCE instance, DWORD reason, void *reserved )
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls( instance );

        /* Callers such as .NET use LoadLibrary success to select ICU instead
         * of NLS, then require its exports. Do not advertise an unusable ICU
         * when the libraries targeted by icu.spec are not installed. */
        if (!(icuuc = LoadLibraryW( L"icuuc68.dll" )))
        {
            WARN( "ICU unavailable: cannot load icuuc68.dll\n" );
            return FALSE;
        }
        if (!(icuin = LoadLibraryW( L"icuin68.dll" )))
        {
            WARN( "ICU unavailable: cannot load icuin68.dll\n" );
            FreeLibrary( icuuc );
            icuuc = NULL;
            return FALSE;
        }
        break;

    case DLL_PROCESS_DETACH:
        if (reserved) break;
        if (icuin) FreeLibrary( icuin );
        if (icuuc) FreeLibrary( icuuc );
        break;
    }
    return TRUE;
}
