#include <common.h>

int DECOMP_CDSYS_XAGetTrackLength(int categoryID, int audioTrackID)
{
	if(sdata->boolUseDisc == 0) return 0;
	if(sdata->bool_XnfLoaded == 0) return 0;
	if(categoryID > 2) return 0;
	if(audioTrackID >= CDSYS_XAGetNumTracks(categoryID)) return 0;

	return sdata->XA_EntrySizeArr[
				sdata->ptrArray_firstSongIndex[categoryID] +
				audioTrackID
			].XaBytes;
}