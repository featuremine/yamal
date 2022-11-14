#if !defined(DECCONTEXTSYMBOLS)
#define DECCONTEXTSYMBOLS

#ifdef IN_LIBGCC2
#define decContextDefault __decContextDefault
#define decContextGetRounding __decContextGetRounding
#define decContextSetRounding __decContextSetRounding
#define DECPOWERS __decPOWERS
#define DECSTICKYTAB __decSTICKYTAB
#endif

#endif
