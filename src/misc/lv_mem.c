/**
 * @file lv_mem.c
 * General and portable implementation of malloc and free.
 * The dynamic memory monitoring is also supported.
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_mem.h"
#include "lv_tlsf.h"
#include "lv_gc.h"
#include "lv_assert.h"
#include "lv_log.h"
#include "am_mcu_apollo.h"

#if LV_MEM_CUSTOM != 0
    #include LV_MEM_CUSTOM_INCLUDE
#endif

#ifdef LV_MEM_POOL_INCLUDE
    #include LV_MEM_POOL_INCLUDE
#endif

/*********************
 *      DEFINES
 *********************/
/*memset the allocated memories to 0xaa and freed memories to 0xbb (just for testing purposes)*/
#ifndef LV_MEM_ADD_JUNK
    #define LV_MEM_ADD_JUNK  0
#endif

#ifdef LV_ARCH_64
    #define MEM_UNIT         uint64_t
    #define ALIGN_MASK       0x7
#else
    #define MEM_UNIT         uint32_t
    #define ALIGN_MASK       0x3
#endif

#define ZERO_MEM_SENTINEL  0xa1b2c3d4

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
#if LV_MEM_CUSTOM == 0
    static void lv_mem_walker(void * ptr, size_t size, int used, void * user);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
#if LV_MEM_CUSTOM == 0
    static lv_tlsf_t tlsf_tcm;
    static lv_tlsf_t tlsf_ssram;
    static lv_tlsf_t tlsf_psram;
#endif

static uint32_t zero_mem = ZERO_MEM_SENTINEL; /*Give the address of this variable if 0 byte should be allocated*/

/**********************
 *      MACROS
 **********************/
#if LV_LOG_TRACE_MEM
    #define MEM_TRACE(...) LV_LOG_TRACE(__VA_ARGS__)
#else
    #define MEM_TRACE(...)
#endif

#define COPY32 *d32 = *s32; d32++; s32++;
#define COPY8 *d8 = *s8; d8++; s8++;
#define SET32(x) *d32 = x; d32++;
#define SET8(x) *d8 = x; d8++;
#define REPEAT8(expr) expr expr expr expr expr expr expr expr

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize the dyn_mem module (work memory and other variables)
 */
void lv_mem_init(void)
{
#if LV_MEM_CUSTOM == 0

#if ( LV_MEM_TCM_ADR == 0 ) || ( !defined(LV_MEM_TCM_SIZE) )
#error "Must specify TCM heap memory address and size"
#else
    tlsf_tcm = lv_tlsf_create_with_pool((void *)LV_MEM_TCM_ADR, LV_MEM_TCM_SIZE);
#endif

#if ( LV_MEM_SSRAM_ADR == 0 ) || ( !defined(LV_MEM_SSRAM_SIZE) )
#error "Must specify SSRAM heap memory address and size"
#else
    tlsf_ssram = lv_tlsf_create_with_pool((void *)LV_MEM_SSRAM_ADR, LV_MEM_SSRAM_SIZE);
#endif

#if ( LV_MEM_PSRAM_ADR == 0 ) || ( !defined(LV_MEM_PSRAM_SIZE) )
#error "Must specify PSRAM heap memory address and size"
#else
    tlsf_psram = lv_tlsf_create_with_pool((void *)LV_MEM_PSRAM_ADR, LV_MEM_PSRAM_SIZE);
#endif

#endif

    for(uint8_t i = 0; i < LV_MEM_BUF_MAX_NUM; i++) {
        LV_GC_ROOT(lv_mem_buf[i]).p = NULL;
        LV_GC_ROOT(lv_mem_buf[i]).size = 0;
        LV_GC_ROOT(lv_mem_buf[i]).used = 0;
    }

#if LV_MEM_ADD_JUNK
    LV_LOG_WARN("LV_MEM_ADD_JUNK is enabled which makes LVGL much slower");
#endif
}

/**
 * Clean up the memory buffer which frees all the allocated memories.
 * @note It work only if `LV_MEM_CUSTOM == 0`
 */
void lv_mem_deinit(void)
{
#if LV_MEM_CUSTOM == 0
    lv_tlsf_destroy(tlsf_tcm);
    lv_tlsf_destroy(tlsf_ssram);
    lv_tlsf_destroy(tlsf_psram);
    lv_mem_init();
#endif
}

/**
 * Allocate a memory dynamically
 * @param size size of the memory to allocate in bytes
 * @return pointer to the allocated memory
 */
void * lv_mem_alloc(size_t size)
{
    MEM_TRACE("allocating %lu bytes", (unsigned long)size);
    if(size == 0) {
        MEM_TRACE("using zero_mem");
        return &zero_mem;
    }

    //Firstly, try TCM
#if LV_MEM_CUSTOM == 0
    void * alloc = lv_tlsf_malloc(tlsf_tcm, size);
#else
    void * alloc = LV_MEM_CUSTOM_TCM_ALLOC(size);
#endif

#ifdef EXPAND_TO_SSRAM 
    // If TCM is used up, try SSRAM
    if(alloc == NULL)
    {
#if LV_MEM_CUSTOM == 0
        alloc = lv_tlsf_malloc(tlsf_ssram, size);
#else
        alloc = LV_MEM_CUSTOM_SSRAM_ALLOC(size);
#endif
    }

    // If SSRAM used up, no need to try PSRAM, it's too slow to satisfy application request.
#endif

    if(alloc == NULL) {
        LV_LOG_ERROR("couldn't allocate memory (%lu bytes)", (unsigned long)size);
        lv_mem_monitor_t mon;
        lv_mem_monitor(&mon);
        LV_LOG_ERROR("used: %6d (%3d %%), frag: %3d %%, biggest free: %6d",
                     (int)(mon.total_size - mon.free_size), mon.used_pct, mon.frag_pct,
                     (int)mon.free_biggest_size);
    }
#if LV_MEM_ADD_JUNK
    else {
        lv_memset(alloc, 0xaa, size);
    }
#endif

    MEM_TRACE("allocated at %p", alloc);
    return alloc;
}

/**
 * Allocate a memory dynamically in SSRAM
 * @param size size of the memory to allocate in bytes
 * @return pointer to the allocated memory
 */
void * lv_mem_ssram_alloc(size_t size)
{
    MEM_TRACE("allocating %lu bytes in ssram", (unsigned long)size);
    if(size == 0) {
        MEM_TRACE("using zero_mem");
        return &zero_mem;
    }

    //Alloc buffer from SSRAM
#if LV_MEM_CUSTOM == 0
    #if defined(AM_PART_APOLLO5A) || defined(AM_PART_APOLLO5B)
        void * alloc = lv_tlsf_memalign(tlsf_ssram, 32, size);
    #else
        void * alloc = lv_tlsf_memalign(tlsf_ssram, 8, size);
    #endif
#else
    void * alloc = LV_MEM_CUSTOM_SSRAM_ALLOC(size);
#endif

    if(alloc == NULL) {
        LV_LOG_ERROR("couldn't allocate memory (%lu bytes)", (unsigned long)size);
        lv_mem_monitor_t mon;
        lv_mem_ssram_monitor(&mon);
        LV_LOG_ERROR("used: %6d (%3d %%), frag: %3d %%, biggest free: %6d",
                     (int)(mon.total_size - mon.free_size), mon.used_pct, mon.frag_pct,
                     (int)mon.free_biggest_size);
    }
#if LV_MEM_ADD_JUNK
    else {
        lv_memset(alloc, 0xaa, size);
    }
#endif

    MEM_TRACE("allocated at %p", alloc);
    return alloc;
}

/**
 * Allocate a memory dynamically in PSRAM
 * @param size size of the memory to allocate in bytes
 * @return pointer to the allocated memory
 */
void * lv_mem_external_alloc(size_t size)
{
    MEM_TRACE("allocating %lu bytes in psram", (unsigned long)size);
    if(size == 0) {
        MEM_TRACE("using zero_mem");
        return &zero_mem;
    }

    //Alloc memory from PSRAM
#if LV_MEM_CUSTOM == 0

    #if defined(AM_PART_APOLLO5A) || defined(AM_PART_APOLLO5B)
        void * alloc = lv_tlsf_memalign(tlsf_psram, 32, size);
    #else
        void * alloc = lv_tlsf_memalign(tlsf_psram, 8, size);
    #endif

#else
    void * alloc = LV_MEM_CUSTOM_PSRAM_ALLOC(size);
#endif

    if(alloc == NULL) {
        LV_LOG_ERROR("couldn't allocate memory (%lu bytes)", (unsigned long)size);
        lv_mem_monitor_t mon;
        lv_mem_external_monitor(&mon);
        LV_LOG_ERROR("used: %6d (%3d %%), frag: %3d %%, biggest free: %6d",
                     (int)(mon.total_size - mon.free_size), mon.used_pct, mon.frag_pct,
                     (int)mon.free_biggest_size);
    }
#if LV_MEM_ADD_JUNK
    else {
        lv_memset(alloc, 0xaa, size);
    }
#endif

    MEM_TRACE("allocated at %p", alloc);
    return alloc;
}

/**
 * Free an allocated data
 * @param data pointer to an allocated memory
 */
void lv_mem_free(void * data)
{
    MEM_TRACE("freeing %p", data);
    if(data == &zero_mem) return;
    if(data == NULL) return;

#if LV_MEM_CUSTOM == 0
#  if LV_MEM_ADD_JUNK
    lv_memset(data, 0xbb, lv_tlsf_block_size(data));
#  endif
#endif

    uint32_t data_u32 = (uint32_t)data;
#if defined(AM_PART_APOLLO5A) || defined(AM_PART_APOLLO5B)
    if( ( data_u32 >= DTCM_BASEADDR ) && ( data_u32 <= (DTCM_BASEADDR + DTCM_MAX_SIZE) ) )
#else
    if( ( data_u32 >= SRAM_BASEADDR ) && ( data_u32 <= (SRAM_BASEADDR + TCM_MAX_SIZE) ) )
#endif
    {
#if LV_MEM_CUSTOM == 0
        lv_tlsf_free(tlsf_tcm, data);
#else
        LV_MEM_CUSTOM_TCM_FREE(data);
#endif
    }
#if defined(AM_PART_APOLLO5A)  || defined(AM_PART_APOLLO5B)
    else if( ( data_u32 >= SSRAM_BASEADDR ) && ( data_u32 <= (SSRAM_BASEADDR + SSRAM_MAX_SIZE) ) )
#else
    else if( ( data_u32 >= SSRAM0_BASEADDR ) && ( data_u32 <= (SSRAM0_BASEADDR + NONTCM_MAX_SIZE) ) )
#endif
    {
#if LV_MEM_CUSTOM == 0
        lv_tlsf_free(tlsf_ssram, data);
#else
        LV_MEM_CUSTOM_SSRAM_FREE(data);
#endif
    }
    else if( ( data_u32 >= MSPI0_APERTURE_START_ADDR ) && ( data_u32 <= MSPI2_APERTURE_END_ADDR ) )
    {
#if LV_MEM_CUSTOM == 0
        lv_tlsf_free(tlsf_psram, data);
#else
        LV_MEM_CUSTOM_SSRAM_FREE(data);
#endif
    }
    else
    {
        LV_LOG_ERROR("couldn't locate corresponding heap!");
    }
}

/**
 * Reallocate a memory with a new size. The old content will be kept.
 * @param data pointer to an allocated memory.
 * Its content will be copied to the new memory block and freed
 * @param new_size the desired new size in byte
 * @return pointer to the new memory
 */
void * lv_mem_realloc(void * data_p, size_t new_size)
{
    MEM_TRACE("reallocating %p with %lu size", data_p, (unsigned long)new_size);
    if(new_size == 0) {
        MEM_TRACE("using zero_mem");
        lv_mem_free(data_p);
        return &zero_mem;
    }

    if((data_p == &zero_mem) || (data_p == NULL)) return lv_mem_alloc(new_size);

    uint32_t data_u32 = (uint32_t)data_p;
    void * new_p = NULL;
#if defined(AM_PART_APOLLO5A)  || defined(AM_PART_APOLLO5B)
    if( ( data_u32 >= DTCM_BASEADDR ) && ( data_u32 <= (DTCM_BASEADDR + DTCM_MAX_SIZE) ) )
#else
    if( ( data_u32 >= SRAM_BASEADDR ) && ( data_u32 <= (SRAM_BASEADDR + TCM_MAX_SIZE) ) )
#endif
    {
#if LV_MEM_CUSTOM == 0
        new_p = lv_tlsf_realloc(tlsf_tcm, data_p, new_size);
#else
        new_p = LV_MEM_CUSTOM_TCM_REALLOC(data_p, new_size);
#endif
    }
#if defined(AM_PART_APOLLO5A)  || defined(AM_PART_APOLLO5B)
    else if ( ( data_u32 >= SSRAM_BASEADDR ) && ( data_u32 <= (SSRAM_BASEADDR + SSRAM_MAX_SIZE) ) )
#else
    else if ( ( data_u32 >= SSRAM0_BASEADDR ) && ( data_u32 <= (SSRAM0_BASEADDR + NONTCM_MAX_SIZE) ) )
#endif
    {
#if LV_MEM_CUSTOM == 0
        new_p = lv_tlsf_realloc(tlsf_ssram, data_p, new_size);
#else
        new_p = LV_MEM_CUSTOM_SSRAM_REALLOC(data_p, new_size);
#endif
    }
    else if( ( data_u32 >= MSPI0_APERTURE_START_ADDR ) && ( data_u32 <= MSPI2_APERTURE_END_ADDR ) )
    {
#if LV_MEM_CUSTOM == 0
        new_p = lv_tlsf_realloc(tlsf_psram, data_p, new_size);
#else
        new_p = LV_MEM_CUSTOM_PSRAM_REALLOC(data_p, new_size);
#endif
    }
    else
    {
        LV_UNUSED(new_p);
        LV_LOG_ERROR("couldn't locate corresponding heap!");
    }

    if(new_p == NULL) {
        LV_LOG_ERROR("couldn't allocate memory");
        return NULL;
    }

    MEM_TRACE("allocated at %p", new_p);
    return new_p;
}

lv_res_t lv_mem_test(void)
{
    if(zero_mem != ZERO_MEM_SENTINEL) {
        LV_LOG_WARN("zero_mem is written");
        return LV_RES_INV;
    }

#if LV_MEM_CUSTOM == 0
    if(lv_tlsf_check(tlsf_tcm)) {
        LV_LOG_WARN("tcm heap failed");
        return LV_RES_INV;
    }

    if(lv_tlsf_check(tlsf_ssram)) {
        LV_LOG_WARN("ssram heap failed");
        return LV_RES_INV;
    }

    if(lv_tlsf_check(tlsf_psram)) {
        LV_LOG_WARN("psram heap failed");
        return LV_RES_INV;
    }

    if(lv_tlsf_check_pool(lv_tlsf_get_pool(tlsf_tcm))) {
        LV_LOG_WARN("tcm pool failed");
        return LV_RES_INV;
    }

    if(lv_tlsf_check_pool(lv_tlsf_get_pool(tlsf_ssram))) {
        LV_LOG_WARN("ssram pool failed");
        return LV_RES_INV;
    }

    if(lv_tlsf_check_pool(lv_tlsf_get_pool(tlsf_psram))) {
        LV_LOG_WARN("psram pool failed");
        return LV_RES_INV;
    }
#endif
    MEM_TRACE("passed");
    return LV_RES_OK;
}

/**
 * Give information about the work memory of dynamic allocation
 * @param mon_p pointer to a lv_mem_monitor_t variable,
 *              the result of the analysis will be stored here
 */
void lv_mem_monitor(lv_mem_monitor_t * mon_p)
{
    /*Init the data*/
    lv_memset(mon_p, 0, sizeof(lv_mem_monitor_t));
#if LV_MEM_CUSTOM == 0
    MEM_TRACE("begin");

#ifdef EXPAND_TO_SSRAM
    lv_tlsf_walk_pool(lv_tlsf_get_pool(tlsf_tcm),   lv_mem_walker, mon_p);
    lv_tlsf_walk_pool(lv_tlsf_get_pool(tlsf_ssram), lv_mem_walker, mon_p);

    mon_p->total_size = LV_MEM_TCM_SIZE + LV_MEM_SSRAM_SIZE;
#else
    lv_tlsf_walk_pool(lv_tlsf_get_pool(tlsf_tcm),   lv_mem_walker, mon_p);

    mon_p->total_size = LV_MEM_TCM_SIZE;
#endif

    mon_p->total_size = LV_MEM_TCM_SIZE;
    mon_p->used_pct = 100 - (100U * mon_p->free_size) / mon_p->total_size;
    if(mon_p->free_size > 0) {
        mon_p->frag_pct = mon_p->free_biggest_size * 100U / mon_p->free_size;
        mon_p->frag_pct = 100 - mon_p->frag_pct;
    }
    else {
        mon_p->frag_pct = 0; /*no fragmentation if all the RAM is used*/
    }

    MEM_TRACE("finished");
#endif
}

/**
 * Give information about the work memory of dynamic allocation
 * @param mon_p pointer to a lv_mem_monitor_t variable,
 *              the result of the analysis will be stored here
 */
void lv_mem_ssram_monitor(lv_mem_monitor_t * mon_p)
{
    /*Init the data*/
    lv_memset(mon_p, 0, sizeof(lv_mem_monitor_t));
#if LV_MEM_CUSTOM == 0
    MEM_TRACE("begin");

    lv_tlsf_walk_pool(lv_tlsf_get_pool(tlsf_ssram),   lv_mem_walker, mon_p);

    mon_p->total_size = LV_MEM_SSRAM_SIZE;
    mon_p->used_pct = 100 - (100U * mon_p->free_size) / mon_p->total_size;
    if(mon_p->free_size > 0) {
        mon_p->frag_pct = mon_p->free_biggest_size * 100U / mon_p->free_size;
        mon_p->frag_pct = 100 - mon_p->frag_pct;
    }
    else {
        mon_p->frag_pct = 0; /*no fragmentation if all the RAM is used*/
    }

    MEM_TRACE("finished");
#endif
}

/**
 * Give information about the work memory of dynamic allocation
 * @param mon_p pointer to a lv_mem_monitor_t variable,
 *              the result of the analysis will be stored here
 */
void lv_mem_external_monitor(lv_mem_monitor_t * mon_p)
{
    /*Init the data*/
    lv_memset(mon_p, 0, sizeof(lv_mem_monitor_t));
#if LV_MEM_CUSTOM == 0
    MEM_TRACE("begin");

    lv_tlsf_walk_pool(lv_tlsf_get_pool(tlsf_psram),   lv_mem_walker, mon_p);

    mon_p->total_size = LV_MEM_PSRAM_SIZE;
    mon_p->used_pct = 100 - (100U * mon_p->free_size) / mon_p->total_size;
    if(mon_p->free_size > 0) {
        mon_p->frag_pct = mon_p->free_biggest_size * 100U / mon_p->free_size;
        mon_p->frag_pct = 100 - mon_p->frag_pct;
    }
    else {
        mon_p->frag_pct = 0; /*no fragmentation if all the RAM is used*/
    }

    MEM_TRACE("finished");
#endif
}

/**
 * Allocate a buf for font glyph
 * @param size size of the font glyph
 * @return pointer to the allocated memory
 * @note if the size is less than 4K bytes, this API will first try to allocate
 * the buffer by lv_mem_ssram_alloc API, if it is bigger than 4K or lv_mem_ssram_alloc failed, it will
 * try allocate buffer by lv_mem_external_alloc API.
 */
void* lv_mem_font_glyph_alloc(uint32_t size)
{
    void* buf_ptr = NULL;

    //If glyph_size_byte is less than 4K, try to alloc the glyph buffer from SSRAM
    if(size <= 4*1024)
    {
        buf_ptr = (uint8_t *)lv_mem_ssram_alloc(size);
    }

    //If glyph_size_byte is bigger than 4K or SSRAM malloc failed, try to alloc it from PSRAM
    if(buf_ptr == NULL)
    {
        buf_ptr = (uint8_t *)lv_mem_external_alloc(size);
    }

    return buf_ptr;
}

/**
 * Allocate a buf for font info
 * @param size size of the font glyph
 * @return pointer to the allocated memory
 * @note if the size is less than 4K bytes, this API will first try to allocate
 * the buffer by lv_mem_alloc API, if it is bigger than 4K or lv_mem_alloc failed, it will
 * try allocate buffer by lv_mem_external_alloc API.
 */
void* lv_mem_font_info_alloc(uint32_t size)
{
    void* buf_ptr = NULL;

    //If glyph_size_byte is less than 4K, try to alloc the glyph buffer from TCM or SSRAM.
    if(size <= 4*1024)
    {
        buf_ptr = (uint8_t *)lv_mem_alloc(size);
    }

    //If glyph_size_byte is bigger than 4K or SSRAM malloc failed, try to alloc it from PSRAM
    if(buf_ptr == NULL)
    {
        buf_ptr = (uint8_t *)lv_mem_external_alloc(size);
    }

    return buf_ptr;
}


/**
 * Get a temporal buffer with the given size.
 * @param size the required size
 */
void * lv_mem_buf_get(uint32_t size)
{
    if(size == 0) return NULL;

    MEM_TRACE("begin, getting %d bytes", size);

    /*Try to find a free buffer with suitable size*/
    int8_t i_guess = -1;
    for(uint8_t i = 0; i < LV_MEM_BUF_MAX_NUM; i++) {
        if(LV_GC_ROOT(lv_mem_buf[i]).used == 0 && LV_GC_ROOT(lv_mem_buf[i]).size >= size) {
            if(LV_GC_ROOT(lv_mem_buf[i]).size == size) {
                LV_GC_ROOT(lv_mem_buf[i]).used = 1;
                return LV_GC_ROOT(lv_mem_buf[i]).p;
            }
            else if(i_guess < 0) {
                i_guess = i;
            }
            /*If size of `i` is closer to `size` prefer it*/
            else if(LV_GC_ROOT(lv_mem_buf[i]).size < LV_GC_ROOT(lv_mem_buf[i_guess]).size) {
                i_guess = i;
            }
        }
    }

    if(i_guess >= 0) {
        LV_GC_ROOT(lv_mem_buf[i_guess]).used = 1;
        MEM_TRACE("returning already allocated buffer (buffer id: %d, address: %p)", i_guess,
                  LV_GC_ROOT(lv_mem_buf[i_guess]).p);
        return LV_GC_ROOT(lv_mem_buf[i_guess]).p;
    }

    /*Reallocate a free buffer*/
    for(uint8_t i = 0; i < LV_MEM_BUF_MAX_NUM; i++) {
        if(LV_GC_ROOT(lv_mem_buf[i]).used == 0) {
            /*if this fails you probably need to increase your LV_MEM_SIZE/heap size*/
            void * buf = lv_mem_realloc(LV_GC_ROOT(lv_mem_buf[i]).p, size);
            LV_ASSERT_MSG(buf != NULL, "Out of memory, can't allocate a new buffer (increase your LV_MEM_SIZE/heap size)");
            if(buf == NULL) return NULL;

            LV_GC_ROOT(lv_mem_buf[i]).used = 1;
            LV_GC_ROOT(lv_mem_buf[i]).size = size;
            LV_GC_ROOT(lv_mem_buf[i]).p    = buf;
            MEM_TRACE("allocated (buffer id: %d, address: %p)", i, LV_GC_ROOT(lv_mem_buf[i]).p);
            return LV_GC_ROOT(lv_mem_buf[i]).p;
        }
    }

    LV_LOG_ERROR("no more buffers. (increase LV_MEM_BUF_MAX_NUM)");
    LV_ASSERT_MSG(false, "No more buffers. Increase LV_MEM_BUF_MAX_NUM.");
    return NULL;
}

/**
 * Release a memory buffer
 * @param p buffer to release
 */
void lv_mem_buf_release(void * p)
{
    MEM_TRACE("begin (address: %p)", p);

    for(uint8_t i = 0; i < LV_MEM_BUF_MAX_NUM; i++) {
        if(LV_GC_ROOT(lv_mem_buf[i]).p == p) {
            LV_GC_ROOT(lv_mem_buf[i]).used = 0;
            return;
        }
    }

    LV_LOG_ERROR("p is not a known buffer");
}

/**
 * Free all memory buffers
 */
void lv_mem_buf_free_all(void)
{
    for(uint8_t i = 0; i < LV_MEM_BUF_MAX_NUM; i++) {
        if(LV_GC_ROOT(lv_mem_buf[i]).p) {
            lv_mem_free(LV_GC_ROOT(lv_mem_buf[i]).p);
            LV_GC_ROOT(lv_mem_buf[i]).p = NULL;
            LV_GC_ROOT(lv_mem_buf[i]).used = 0;
            LV_GC_ROOT(lv_mem_buf[i]).size = 0;
        }
    }
}

#if LV_MEMCPY_MEMSET_STD == 0
/**
 * Same as `memcpy` but optimized for 4 byte operation.
 * @param dst pointer to the destination buffer
 * @param src pointer to the source buffer
 * @param len number of byte to copy
 */
LV_ATTRIBUTE_FAST_MEM void * lv_memcpy(void * dst, const void * src, size_t len)
{
    uint8_t * d8 = dst;
    const uint8_t * s8 = src;

    lv_uintptr_t d_align = (lv_uintptr_t)d8 & ALIGN_MASK;
    lv_uintptr_t s_align = (lv_uintptr_t)s8 & ALIGN_MASK;

    /*Byte copy for unaligned memories*/
    if(s_align != d_align) {
        while(len > 32) {
            REPEAT8(COPY8);
            REPEAT8(COPY8);
            REPEAT8(COPY8);
            REPEAT8(COPY8);
            len -= 32;
        }
        while(len) {
            COPY8
            len--;
        }
        return dst;
    }

    /*Make the memories aligned*/
    if(d_align) {
        d_align = ALIGN_MASK + 1 - d_align;
        while(d_align && len) {
            COPY8;
            d_align--;
            len--;
        }
    }

    uint32_t * d32 = (uint32_t *)d8;
    const uint32_t * s32 = (uint32_t *)s8;
    while(len > 32) {
        REPEAT8(COPY32)
        len -= 32;
    }

    while(len > 4) {
        COPY32;
        len -= 4;
    }

    d8 = (uint8_t *)d32;
    s8 = (const uint8_t *)s32;
    while(len) {
        COPY8
        len--;
    }

    return dst;
}

/**
 * Same as `memset` but optimized for 4 byte operation.
 * @param dst pointer to the destination buffer
 * @param v value to set [0..255]
 * @param len number of byte to set
 */
LV_ATTRIBUTE_FAST_MEM void lv_memset(void * dst, uint8_t v, size_t len)
{

    uint8_t * d8 = (uint8_t *)dst;

    uintptr_t d_align = (lv_uintptr_t) d8 & ALIGN_MASK;

    /*Make the address aligned*/
    if(d_align) {
        d_align = ALIGN_MASK + 1 - d_align;
        while(d_align && len) {
            SET8(v);
            len--;
            d_align--;
        }
    }

    uint32_t v32 = (uint32_t)v + ((uint32_t)v << 8) + ((uint32_t)v << 16) + ((uint32_t)v << 24);

    uint32_t * d32 = (uint32_t *)d8;

    while(len > 32) {
        REPEAT8(SET32(v32));
        len -= 32;
    }

    while(len > 4) {
        SET32(v32);
        len -= 4;
    }

    d8 = (uint8_t *)d32;
    while(len) {
        SET8(v);
        len--;
    }
}

/**
 * Same as `memset(dst, 0x00, len)` but optimized for 4 byte operation.
 * @param dst pointer to the destination buffer
 * @param len number of byte to set
 */
LV_ATTRIBUTE_FAST_MEM void lv_memset_00(void * dst, size_t len)
{
    uint8_t * d8 = (uint8_t *)dst;
    uintptr_t d_align = (lv_uintptr_t) d8 & ALIGN_MASK;

    /*Make the address aligned*/
    if(d_align) {
        d_align = ALIGN_MASK + 1 - d_align;
        while(d_align && len) {
            SET8(0);
            len--;
            d_align--;
        }
    }

    uint32_t * d32 = (uint32_t *)d8;
    while(len > 32) {
        REPEAT8(SET32(0));
        len -= 32;
    }

    while(len > 4) {
        SET32(0);
        len -= 4;
    }

    d8 = (uint8_t *)d32;
    while(len) {
        SET8(0);
        len--;
    }
}

/**
 * Same as `memset(dst, 0xFF, len)` but optimized for 4 byte operation.
 * @param dst pointer to the destination buffer
 * @param len number of byte to set
 */
LV_ATTRIBUTE_FAST_MEM void lv_memset_ff(void * dst, size_t len)
{
    uint8_t * d8 = (uint8_t *)dst;
    uintptr_t d_align = (lv_uintptr_t) d8 & ALIGN_MASK;

    /*Make the address aligned*/
    if(d_align) {
        d_align = ALIGN_MASK + 1 - d_align;
        while(d_align && len) {
            SET8(0xFF);
            len--;
            d_align--;
        }
    }

    uint32_t * d32 = (uint32_t *)d8;
    while(len > 32) {
        REPEAT8(SET32(0xFFFFFFFF));
        len -= 32;
    }

    while(len > 4) {
        SET32(0xFFFFFFFF);
        len -= 4;
    }

    d8 = (uint8_t *)d32;
    while(len) {
        SET8(0xFF);
        len--;
    }
}

#endif /*LV_MEMCPY_MEMSET_STD*/

/**********************
 *   STATIC FUNCTIONS
 **********************/

#if LV_MEM_CUSTOM == 0
static void lv_mem_walker(void * ptr, size_t size, int used, void * user)
{
    LV_UNUSED(ptr);

    lv_mem_monitor_t * mon_p = user;
    if(used) {
        mon_p->used_cnt++;
    }
    else {
        mon_p->free_cnt++;
        mon_p->free_size += size;
        if(size > mon_p->free_biggest_size)
            mon_p->free_biggest_size = size;
    }
}
#endif
