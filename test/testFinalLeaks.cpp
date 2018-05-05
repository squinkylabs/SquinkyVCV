
#include "ThreadSharedState.h"
#include "ThreadServer.h"
#include "FFTData.h"

#include "asserts.h"

void testFinalLeaks()
{
    assertEQ(ThreadMessage::_dbgCount, 0);
    assertEQ(FFTDataReal::_count, 0);
    assertEQ(FFTDataCpx::_count, 0);
    assertEQ(ThreadSharedState::_dbgCount, 0);
    assertEQ(ThreadServer::_count, 0);
}