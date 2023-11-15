#include <common.h>

void DECOMP_MM_MENUBOX_Difficulty(struct MenuBox* mb)
{
  short row;
  
  row = mb->rowSelected;
  
  // if uninitialized
  if (row == -1) {
    mb->ptrPrevBox_InHierarchy->state &= ~(ONLY_DRAW_TITLE | DRAW_NEXT_MENU_IN_HIERARCHY);
  }
  
  else 
  {
	// if you are on a valid row
    if ((unsigned char)row < 3) 
	{
	  // set difficulty to value, from array of fixed difficulty values
      sdata->gGT->arcadeDifficulty = D230.cupDifficultySpeed[row];
      
	  D230.MM_State = 2;
	  D230.desiredMenu = 2;
      
	  mb->state |= 4;
      return;
    }
  }
  return;
}
 