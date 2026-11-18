/* Bounded SacredReborn wrapped-Zink swap diagnostics. No rendering changes. */
#ifndef __WINE_SWAP_PROFILE_H
#define __WINE_SWAP_PROFILE_H

#include <time.h>

struct swap_profile_time
{
    ULONGLONG wall, cpu;
};

static __thread struct
{
    BOOL checked, enabled, active;
    unsigned int reports, calls, failed;
    struct swap_profile_time start, call;
    struct
    {
        const char *name;
        unsigned int count;
        ULONGLONG wall, cpu, maximum;
    } phase[8];
    ULONGLONG wall, cpu;
} swap_profile;

static struct swap_profile_time swap_profile_now(void)
{
    struct timespec wall = {0}, cpu = {0};
    struct swap_profile_time result;

    clock_gettime( CLOCK_MONOTONIC, &wall );
    clock_gettime( CLOCK_THREAD_CPUTIME_ID, &cpu );
    result.wall = (ULONGLONG)wall.tv_sec * 1000000000 + wall.tv_nsec;
    result.cpu = (ULONGLONG)cpu.tv_sec * 1000000000 + cpu.tv_nsec;
    return result;
}

static struct swap_profile_time swap_profile_mark(void)
{
    struct swap_profile_time zero = {0};
    return swap_profile.active ? swap_profile_now() : zero;
}

static void swap_profile_add( unsigned int phase, const char *name, struct swap_profile_time start )
{
    struct swap_profile_time now;
    ULONGLONG elapsed;

    if (!swap_profile.active) return;
    now = swap_profile_now();
    elapsed = now.wall - start.wall;
    swap_profile.phase[phase].name = name;
    swap_profile.phase[phase].count++;
    swap_profile.phase[phase].wall += elapsed;
    swap_profile.phase[phase].cpu += now.cpu - start.cpu;
    if (elapsed > swap_profile.phase[phase].maximum) swap_profile.phase[phase].maximum = elapsed;
}

static void swap_profile_begin( const struct opengl_funcs *funcs )
{
    if (!swap_profile.checked)
    {
        const WCHAR *p, *name = NtCurrentTeb()->Peb->ProcessParameters->ImagePathName.Buffer;
        const char *renderer;

        if (!NtCurrentTeb()->glContext) return;
        swap_profile.checked = TRUE;
        if (sizeof(void *) != 4 && !NtCurrentTeb()->WowTebOffset) return;
        if ((p = wcsrchr( name, '/' ))) name = p + 1;
        if ((p = wcsrchr( name, '\\' ))) name = p + 1;
        if (wcsicmp( name, L"SacredReborn.exe" )) return;
        renderer = (const char *)funcs->p_glGetString( GL_RENDERER );
        if (!renderer || !strstr( renderer, "zink" ) || !strstr( renderer, "Wrapper" )) return;
        swap_profile.enabled = TRUE;
        MESSAGE( "SWAP_PROFILE enabled for wrapped Zink: elapsed and thread CPU timing, at most 60 windows.\n" );
    }
    if (!(swap_profile.active = swap_profile.enabled)) return;
    swap_profile.call = swap_profile_now();
    if (!swap_profile.start.wall) swap_profile.start = swap_profile.call;
}

static void swap_profile_end( const char *label, BOOL success, int interval, int offscreen )
{
    struct swap_profile_time now;
    unsigned int i;

    if (!swap_profile.active) return;
    now = swap_profile_now();
    swap_profile.active = FALSE;
    swap_profile.calls++;
    if (!success) swap_profile.failed++;
    swap_profile.wall += now.wall - swap_profile.call.wall;
    swap_profile.cpu += now.cpu - swap_profile.call.cpu;
    if (now.wall - swap_profile.start.wall < 5000000000ULL) return;

    MESSAGE( "%s thread=%p window=%u elapsed_ms=%.3f calls=%u failed=%u call_wall_ms=%.3f "
             "call_cpu_ms=%.3f interval=%d offscreen=%d nested_phases=1\n", label,
             NtCurrentTeb()->ClientId.UniqueThread, ++swap_profile.reports,
             (now.wall - swap_profile.start.wall) / 1000000.0, swap_profile.calls, swap_profile.failed,
             swap_profile.wall / 1000000.0, swap_profile.cpu / 1000000.0, interval, offscreen );
    for (i = 0; i < ARRAY_SIZE(swap_profile.phase); i++)
        if (swap_profile.phase[i].count)
            MESSAGE( "%s thread=%p window=%u phase=%s count=%u wall_ms=%.3f cpu_ms=%.3f max_ms=%.3f\n",
                     label, NtCurrentTeb()->ClientId.UniqueThread, swap_profile.reports,
                     swap_profile.phase[i].name, swap_profile.phase[i].count,
                     swap_profile.phase[i].wall / 1000000.0, swap_profile.phase[i].cpu / 1000000.0,
                     swap_profile.phase[i].maximum / 1000000.0 );
    memset( swap_profile.phase, 0, sizeof(swap_profile.phase) );
    swap_profile.calls = swap_profile.failed = 0;
    swap_profile.wall = swap_profile.cpu = 0;
    swap_profile.start = swap_profile_now();
    if (swap_profile.reports == 60)
    {
        swap_profile.enabled = FALSE;
        MESSAGE( "%s complete; timing disabled.\n", label );
    }
}

#endif
