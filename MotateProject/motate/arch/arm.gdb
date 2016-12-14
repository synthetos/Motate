# Inspired by https://github.com/bnahill/PyCortexMDebug/blob/master/cmdebug/dwt_gdb.py

set $DWT_CTRL     = 0xE0001000
set $DWT_CYCCNT   = 0xE0001004
set $DWT_CPICNT   = 0xE0001008
set $DWT_EXTCNT   = 0xE000100C
set $DWT_SLEEPCNT = 0xE0001010
set $DWT_LSUCNT   = 0xE0001014
set $DWT_FOLDCNT  = 0xE0001018
set $DWT_PCSR     = 0xE000101C


define dwt_cyccnt_en
  set *(uint32_t *)$DWT_CTRL = *(uint32_t *)$DWT_CTRL | 1
end

define dwt_cyccnt_dis
    set *(uint32_t *)$DWT_CTRL = *(uint32_t *)$DWT_CTRL & ~1
end

define dwt_cyccnt
    print *(uint32_t *)$DWT_CYCCNT
end

defin dwt_time
    print (float)(*(uint32_t *)$DWT_CYCCNT) / (float)(SystemCoreClock / 1000)
end

defin dwt_time_us
    print (float)(*(uint32_t *)$DWT_CYCCNT) / (float)(SystemCoreClock / 1000000)
end

define dwt_reset
    set *(uint32_t *)$DWT_CYCCNT = 0
end

echo **DWT commands enabled**\n
