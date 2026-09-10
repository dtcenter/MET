#include <type_traits>
#include "pair_stat_conf_info.h"

#define CHECK_NOTHROW_MOVE(Type) \
    static_assert(std::is_nothrow_move_constructible<Type>::value, \
                  #Type " is NOT nothrow move constructible");

CHECK_NOTHROW_MOVE(ConcatString)
CHECK_NOTHROW_MOVE(NumArray)
CHECK_NOTHROW_MOVE(IntArray)
CHECK_NOTHROW_MOVE(TimeArray)
CHECK_NOTHROW_MOVE(StringArray)
CHECK_NOTHROW_MOVE(ThreshArray)
CHECK_NOTHROW_MOVE(Grid)

CHECK_NOTHROW_MOVE(VxPairDataPoint)
CHECK_NOTHROW_MOVE(StatHdrInfo)
CHECK_NOTHROW_MOVE(SetLogic)
CHECK_NOTHROW_MOVE(MaskLatLon)
CHECK_NOTHROW_MOVE(ClimoCDFInfo)
CHECK_NOTHROW_MOVE(BootInfo)
