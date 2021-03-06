#ifndef NILE_H
#define NILE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct nile_Process_ nile_Process_t;

/* Runtime management */

nile_Process_t *
nile_startup (char *memory, int nbytes, int nthreads);

char *
nile_shutdown (nile_Process_t *init);

void
nile_sync (nile_Process_t *init);

typedef enum {
    NILE_STATUS_OK,
    NILE_STATUS_OUT_OF_MEMORY,
    NILE_STATUS_BAD_ARG,
    NILE_STATUS_SHUTTING_DOWN
} nile_Status_t;

nile_Status_t
nile_status (nile_Process_t *init);

bool
nile_error (nile_Process_t *init);

void
nile_print_leaks (nile_Process_t *init);

/* Connecting and launching process pipelines */

#define NILE_NULL ((void *) 0)

nile_Process_t *
nile_Process_pipe (nile_Process_t *p1, ...);

void
nile_Process_feed (nile_Process_t *p, float *data, int n);

void
nile_Process_gate (nile_Process_t *gater, nile_Process_t *gatee);

nile_Process_t *
nile_Process_pipeline (nile_Process_t **ps, int n);

void
nile_Process_launch (nile_Process_t **ps, int nps, float *data, int ndata);

/* Built-in processes */

nile_Process_t *
nile_PassThrough (nile_Process_t *parent, int quantum);

nile_Process_t *
nile_Capture (nile_Process_t *parent, float *data, int *n, int size);

nile_Process_t *
nile_Reverse (nile_Process_t *parent, int in_quantum);

nile_Process_t *
nile_SortBy (nile_Process_t *p, int quantum, int index);

nile_Process_t *
nile_DupZip (nile_Process_t *p,  int quantum,
             nile_Process_t *p1, int p1_out_quantum,
             nile_Process_t *p2, int p2_out_quantum);

nile_Process_t *
nile_DupCat (nile_Process_t *p,  int quantum,
             nile_Process_t *p1, int p1_out_quantum,
             nile_Process_t *p2, int p2_out_quantum);

nile_Process_t *
nile_Funnel (nile_Process_t *parent);

void
nile_Funnel_pour (nile_Process_t *p, float *data, int n, int EOS);

#ifdef NILE_INCLUDE_PROCESS_API

#ifdef _MSC_VER
#define INLINE static __forceinline
#else
#define INLINE static inline
#endif

/* Real numbers */

#include <math.h>

#define Real nile_Real_t

typedef struct { float f; } nile_Real_t;

INLINE Real  nile_Real     (float f)        { Real r = {f}; return r;        }
INLINE int   nile_Real_nz  (Real a)         { return  a.f != 0;              }
INLINE int   nile_Real_toi (Real a)         { return (int) a.f;              }
INLINE float nile_Real_tof (Real a)         { return       a.f;              }
INLINE Real  nile_Real_neg (Real a)         { return nile_Real (-a.f);       }
INLINE Real  nile_Real_sqt (Real a)         { return nile_Real (sqrtf(a.f)); }
INLINE Real  nile_Real_add (Real a, Real b) { return nile_Real (a.f +  b.f); }
INLINE Real  nile_Real_sub (Real a, Real b) { return nile_Real (a.f -  b.f); }
INLINE Real  nile_Real_mul (Real a, Real b) { return nile_Real (a.f *  b.f); }
INLINE Real  nile_Real_div (Real a, Real b) { return nile_Real (a.f /  b.f); }
INLINE Real  nile_Real_eq  (Real a, Real b) { return nile_Real (a.f == b.f); }
INLINE Real  nile_Real_neq (Real a, Real b) { return nile_Real (a.f != b.f); }
INLINE Real  nile_Real_lt  (Real a, Real b) { return nile_Real (a.f <  b.f); }
INLINE Real  nile_Real_gt  (Real a, Real b) { return nile_Real (a.f >  b.f); }
INLINE Real  nile_Real_leq (Real a, Real b) { return nile_Real (a.f <= b.f); }
INLINE Real  nile_Real_geq (Real a, Real b) { return nile_Real (a.f >= b.f); }
INLINE Real  nile_Real_or  (Real a, Real b) { return nile_Real (a.f || b.f); }
INLINE Real  nile_Real_and (Real a, Real b) { return nile_Real (a.f && b.f); }
INLINE Real  nile_Real_flr (Real a)         { Real b = nile_Real ((int)a.f);
                                              return nile_Real
                                                (b.f > a.f ? b.f - 1 : b.f); }
INLINE Real  nile_Real_clg (Real a)         { Real b = nile_Real ((int)a.f);
                                              return nile_Real
                                                (b.f < a.f ? b.f + 1 : b.f); }

/* Stream buffers */

typedef enum {
    NILE_TAG_NONE,
    NILE_TAG_QUOTA_HIT,
    NILE_TAG_OOM,
} nile_Tag_t;

typedef struct {
    int        head;
    int        tail;
    int        capacity;
    nile_Tag_t tag;
    Real       data;
} nile_Buffer_t;

INLINE void nile_Buffer_push_head (nile_Buffer_t *b, Real r) { (&b->data)[--b->head] = r;    }
INLINE void nile_Buffer_push_tail (nile_Buffer_t *b, Real r) { (&b->data)[b->tail++] = r;    }
INLINE Real nile_Buffer_pop_head  (nile_Buffer_t *b)         { return (&b->data)[b->head++]; }
INLINE int  nile_Buffer_headroom  (nile_Buffer_t *b)         { return b->head;               }
INLINE int  nile_Buffer_tailroom  (nile_Buffer_t *b)         { return b->capacity - b->tail; }
INLINE int  nile_Buffer_is_empty  (nile_Buffer_t *b)         { return b->head == b->tail;    }
INLINE int  nile_Buffer_quota_hit (nile_Buffer_t *b)
    { return (b->tag == NILE_TAG_QUOTA_HIT) && (nile_Buffer_tailroom (b) < b->capacity / 2); }

/* Process definition API */

typedef nile_Buffer_t *
(*nile_Process_logue_t) (nile_Process_t *p, nile_Buffer_t *out);

typedef nile_Buffer_t *
(*nile_Process_body_t)  (nile_Process_t *p, nile_Buffer_t *in, nile_Buffer_t *out);

nile_Process_t *
nile_Process (nile_Process_t *p, int quantum, int sizeof_vars,
              nile_Process_logue_t prologue,
              nile_Process_body_t  body,
              nile_Process_logue_t epilogue);

void *
nile_Process_vars (nile_Process_t *p);

void *
nile_Process_memory (nile_Process_t *p);

void
nile_Process_advance_output (nile_Process_t *p, float **out, int *j, int *n);

int
nile_Process_advance_input (nile_Process_t *p, float **in, int *i, int *m);

nile_Buffer_t *
nile_Process_append_output (nile_Process_t *p, nile_Buffer_t *out);

nile_Buffer_t *
nile_Process_prefix_input (nile_Process_t *producer, nile_Buffer_t *in);

nile_Buffer_t *
nile_Process_swap (nile_Process_t *p, nile_Process_t *sub, nile_Buffer_t *out);

int
nile_Process_return (nile_Process_t *p, int i, int j, int status);

int
nile_Process_reroute (nile_Process_t *p, int i, int j, nile_Process_t *sub);

#endif
#ifdef __cplusplus
}
#endif
#endif
