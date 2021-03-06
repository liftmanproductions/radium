/* Copyright 2016 Kjetil S. Matheussen

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. */

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>


#include "../common/includepython.h"

#include "../common/nsmtracker.h"
#include "../common/seqtrack_proc.h"
#include "../common/seqtrack_automation_proc.h"
#include "../common/song_tempo_automation_proc.h"
#include "../common/time_proc.h"
#include "../common/undo_sequencer_proc.h"
#include "../common/undo_song_tempo_automation_proc.h"
#include "../common/visual_proc.h"
#include "../common/OS_Bs_edit_proc.h"
#include "../common/settings_proc.h"
#include "../common/visual_proc.h"

#include "../audio/Mixer_proc.h"

#include "api_common_proc.h"

#include "radium_proc.h"

extern struct TEvent tevent;



// sequencer

float getSequencerX1(void){
  return SEQUENCER_get_x1();
}

float getSequencerX2(void){
  return SEQUENCER_get_x2();
}

float getSequencerY1(void){
  return SEQUENCER_get_y1();
}

float getSequencerY2(void){
  return SEQUENCER_get_y2();
}

void undoSequencer(void){
  ADD_UNDO(Sequencer());
}

// sequencer

int64_t getSequencerSongLengthInFrames(void){
  return (SONG_get_length() + SEQUENCER_EXTRA_SONG_LENGTH) * MIXER_get_sample_rate();
}

int64_t getSequencerVisibleStartTime(void){
  return SEQUENCER_get_visible_start_time();
}

int64_t getSequencerVisibleEndTime(void){
  return SEQUENCER_get_visible_end_time();
}

void setSequencerVisibleStartTime(int64_t value){
  //printf("                   Set: %f\n", value/48000.0);
  SEQUENCER_set_visible_start_time(value);
}

void setSequencerVisibleEndTime(int64_t value){
  SEQUENCER_set_visible_end_time(value);
}

void setSequencerGridType(int grid_type){
  SEQUENCER_set_grid_type(grid_type);
}

void setSequencerSelectionRectangle(float x1, float y1, float x2, float y2){
  SEQUENCER_set_selection_rectangle(x1, y1, x2, y2);
}

void unsetSequencerSelectionRectangle(void){
  SEQUENCER_unset_selection_rectangle();
}



float getSeqnavX1(void){
  return SEQNAV_get_x1();
}

float getSeqnavX2(void){
  return SEQNAV_get_x2();
}

float getSeqnavY1(void){
  return SEQNAV_get_y1();
}

float getSeqnavY2(void){
  return SEQNAV_get_y2();
}


float getSeqnavLeftSizeHandleX1(void){
  return SEQNAV_get_left_handle_x();
}

float getSeqnavLeftSizeHandleX2(void){
  return SEQNAV_get_left_handle_x() + SEQNAV_SIZE_HANDLE_WIDTH;
}

float getSeqnavLeftSizeHandleY1(void){
  return getSeqnavY1();
}

float getSeqnavLeftSizeHandleY2(void){
  return getSeqnavY2();
}

float getSeqnavRightSizeHandleX1(void){
  return SEQNAV_get_right_handle_x() - SEQNAV_SIZE_HANDLE_WIDTH;
}

float getSeqnavRightSizeHandleX2(void){
  return SEQNAV_get_right_handle_x();
}

float getSeqnavRightSizeHandleY1(void){
  return getSeqnavY1();
}

float getSeqnavRightSizeHandleY2(void){
  return getSeqnavY2();
}

void appendSeqtrack(void){
  undoSequencer();
  SEQUENCER_append_seqtrack(NULL);

  ATOMIC_SET(root->song->curr_seqtracknum, root->song->seqtracks.num_elements -1);
  BS_UpdatePlayList();
}

void insertSeqtrack(int pos){
  if (pos==-1)
    pos = ATOMIC_GET(root->song->curr_seqtracknum);
  
  if (pos < 0 || pos > root->song->seqtracks.num_elements){
    handleError("Position #%d not legal", pos);
    return;
  }

  undoSequencer();
  SEQUENCER_insert_seqtrack(NULL, pos);

  ATOMIC_SET(root->song->curr_seqtracknum, pos);
  BS_UpdatePlayList();
}

void deleteSeqtrack(int seqtracknum){
  if (seqtracknum==-1)
    seqtracknum = ATOMIC_GET(root->song->curr_seqtracknum);
  
  if (seqtracknum < 0 || seqtracknum >= root->song->seqtracks.num_elements){
    handleError("Sequencer track #%d does not exist", seqtracknum);
    return;
  }

  if (root->song->seqtracks.num_elements==1){
    handleError("Must have at least one sequencer track");
    return;
  }    
  
  undoSequencer();
  SEQUENCER_delete_seqtrack(seqtracknum);
}

void selectSeqtrack(int seqtracknum){
  if (seqtracknum < 0 || seqtracknum >= root->song->seqtracks.num_elements){
    handleError("Sequencer track #%d does not exist", seqtracknum);
    return;
  }

  ATOMIC_SET(root->song->curr_seqtracknum, seqtracknum);
  BS_UpdatePlayList();
  SEQUENCER_update();
}

int getCurrSeqtrack(void){
  return ATOMIC_GET(root->song->curr_seqtracknum);
}

int getNumSeqtracks(void){
  return root->song->seqtracks.num_elements;
}




// Sequencer track automation
//////////////////////////////////////////

int addSeqAutomation(int64_t time1, float value1, int64_t time2, float value2, int effect_num, int64_t instrument_id, int seqtracknum){
  struct SeqTrack *seqtrack = getSeqtrackFromNum(seqtracknum);
  if (seqtrack==NULL)
    return -1;

  struct Patch *patch = getAudioPatchFromNum(instrument_id);
  if(patch==NULL)
    return -1;

  undoSequencer();

  int64_t seqtime1 = get_seqtime_from_abstime(seqtrack, NULL, time1);
  int64_t seqtime2 = get_seqtime_from_abstime(seqtrack, NULL, time2);

  return SEQTRACK_AUTOMATION_add_automation(seqtrack->seqtrackautomation, patch, effect_num, seqtime1, value1, LOGTYPE_LINEAR, seqtime2, value2);
}

void replaceAllSeqAutomation(int64_t old_instrument, int64_t new_instrument){
  struct Patch *old_patch = getAudioPatchFromNum(old_instrument);
  if(old_patch==NULL)
    return;

  struct Patch *new_patch = getAudioPatchFromNum(new_instrument);
  if(new_patch==NULL)
    return;

  undoSequencer();

  SEQTRACK_AUTOMATION_replace_all_automations(old_patch, new_patch);
}

int getNumSeqAutomations(int seqtracknum){
  struct SeqTrack *seqtrack = getSeqtrackFromNum(seqtracknum);
  if (seqtrack==NULL)
    return 0;

  return SEQTRACK_AUTOMATION_get_num_automations(seqtrack->seqtrackautomation);
}

#define VALIDATE_AUTOMATIONNUM(ret)                                     \
  if (automationnum < 0 || automationnum >= SEQTRACK_AUTOMATION_get_num_automations(seqtrack->seqtrackautomation)){ \
    handleError("There is no automation #%d in sequencer track #%d", automationnum, seqtracknum); \
    return ret;                                                         \
  }


#define VALIDATE_NODENUM(ret)                                           \
  if (nodenum < 0 || nodenum >= SEQTRACK_AUTOMATION_get_num_nodes(seqtrack->seqtrackautomation, automationnum)){ \
    handleError("There is no node #%d in automation #%d in sequencer track #%d", nodenum, automationnum, seqtracknum); \
    return ret;                                                          \
  }

int64_t getSeqAutomationInstrumentId(int automationnum, int seqtracknum){
  struct SeqTrack *seqtrack = getSeqtrackFromNum(seqtracknum);
  if (seqtrack==NULL)
    return -1;

  VALIDATE_AUTOMATIONNUM(-1);

  struct Patch *patch = SEQTRACK_AUTOMATION_get_patch(seqtrack->seqtrackautomation, automationnum);
  if (patch==NULL)
    return 0;

  return patch->id;
}

int getSeqAutomationEffectNum(int automationnum, int seqtracknum){
  struct SeqTrack *seqtrack = getSeqtrackFromNum(seqtracknum);
  if (seqtrack==NULL)
    return -1;

  VALIDATE_AUTOMATIONNUM(-1);

  return SEQTRACK_AUTOMATION_get_effect_num(seqtrack->seqtrackautomation, automationnum);
}

float getSeqAutomationValue(int nodenum, int automationnum, int seqtracknum){
  struct SeqTrack *seqtrack = getSeqtrackFromNum(seqtracknum);
  if (seqtrack==NULL)
    return -1;

  VALIDATE_AUTOMATIONNUM(-1);
  VALIDATE_NODENUM(-1);

  return SEQTRACK_AUTOMATION_get_value(seqtrack->seqtrackautomation, automationnum, nodenum);
}

int64_t getSeqAutomationTime(int nodenum, int automationnum, int seqtracknum){
  struct SeqTrack *seqtrack = getSeqtrackFromNum(seqtracknum);
  if (seqtrack==NULL)
    return -1;

  VALIDATE_AUTOMATIONNUM(-1);
  VALIDATE_NODENUM(-1);

  int64_t seqtime = SEQTRACK_AUTOMATION_get_seqtime(seqtrack->seqtrackautomation, automationnum, nodenum);

  return get_abstime_from_seqtime(seqtrack, NULL, seqtime);
}

int getSeqAutomationLogtype(int nodenum, int automationnum, int seqtracknum){
  struct SeqTrack *seqtrack = getSeqtrackFromNum(seqtracknum);
  if (seqtrack==NULL)
    return -1;

  VALIDATE_AUTOMATIONNUM(-1);
  VALIDATE_NODENUM(-1);

  return SEQTRACK_AUTOMATION_get_logtype(seqtrack->seqtrackautomation, automationnum, nodenum);
}

int getNumSeqAutomationNodes(int automationnum, int seqtracknum){
  struct SeqTrack *seqtrack = getSeqtrackFromNum(seqtracknum);
  if (seqtrack==NULL)
    return -1;

  VALIDATE_AUTOMATIONNUM(-1);

  return SEQTRACK_AUTOMATION_get_num_nodes(seqtrack->seqtrackautomation, automationnum);
}

int addSeqAutomationNode(int64_t time, float value, int logtype, int automationnum, int seqtracknum){
  struct SeqTrack *seqtrack = getSeqtrackFromNum(seqtracknum);
  if (seqtrack==NULL)
    return -1;

  VALIDATE_AUTOMATIONNUM(-1);

  undoSequencer();

  int64_t seqtime = get_seqtime_from_abstime(seqtrack, NULL, time);
  return SEQTRACK_AUTOMATION_add_node(seqtrack->seqtrackautomation, automationnum, seqtime, value, logtype);
}

void deleteSeqAutomationNode(int nodenum, int automationnum, int seqtracknum){
  struct SeqTrack *seqtrack = getSeqtrackFromNum(seqtracknum);
  if (seqtrack==NULL)
    return;

  VALIDATE_AUTOMATIONNUM();
  VALIDATE_NODENUM();

  undoSequencer();

  SEQTRACK_AUTOMATION_delete_node(seqtrack->seqtrackautomation, automationnum, nodenum);
}

void setCurrSeqAutomationNode(int nodenum, int automationnum, int seqtracknum){
  struct SeqTrack *seqtrack = getSeqtrackFromNum(seqtracknum);
  if (seqtrack==NULL)
    return;

  VALIDATE_AUTOMATIONNUM();
  VALIDATE_NODENUM();

  SEQTRACK_AUTOMATION_set_curr_node(seqtrack->seqtrackautomation, automationnum, nodenum);
}

void cancelCurrSeqAutomationNode(int automationnum, int seqtracknum){
  struct SeqTrack *seqtrack = getSeqtrackFromNum(seqtracknum);
  if (seqtrack==NULL)
    return;

  VALIDATE_AUTOMATIONNUM();

  SEQTRACK_AUTOMATION_cancel_curr_node(seqtrack->seqtrackautomation, automationnum);
}

void setCurrSeqAutomation(int automationnum, int seqtracknum){
  struct SeqTrack *seqtrack = getSeqtrackFromNum(seqtracknum);
  if (seqtrack==NULL)
    return;

  VALIDATE_AUTOMATIONNUM();

  SEQTRACK_AUTOMATION_set_curr_automation(seqtrack->seqtrackautomation, automationnum);
}

void cancelCurrSeqAutomation(void){
  SEQTRACK_AUTOMATION_cancel_curr_automation();
}

int getCurrSeqAutomationSeqtrack(void){
  int ret = 0;

  ALL_SEQTRACKS_FOR_EACH(){

    if (SEQTRACK_AUTOMATION_get_curr_automation(seqtrack->seqtrackautomation) != -1)
      return ret;

    ret++;

  }END_ALL_SEQTRACKS_FOR_EACH;

  return -1;

}

int getCurrSeqAutomation(void){
  ALL_SEQTRACKS_FOR_EACH(){

    int maybe = SEQTRACK_AUTOMATION_get_curr_automation(seqtrack->seqtrackautomation);

    if (maybe != -1)
      return maybe;

  }END_ALL_SEQTRACKS_FOR_EACH;

  return -1;
}

void setSeqAutomationNode(int64_t time, float value, int logtype, int nodenum, int automationnum, int seqtracknum){
  struct SeqTrack *seqtrack = getSeqtrackFromNum(seqtracknum);
  if (seqtrack==NULL)
    return;

  VALIDATE_AUTOMATIONNUM();
  VALIDATE_NODENUM();

  int64_t seqtime = get_seqtime_from_abstime(seqtrack, NULL, time);
  SEQTRACK_AUTOMATION_set(seqtrack, automationnum, nodenum, seqtime, R_BOUNDARIES(0, value, 1), logtype);
}

float getSeqAutomationNodeX(int nodenum, int automationnum, int seqtracknum){
  struct SeqTrack *seqtrack = getSeqtrackFromNum(seqtracknum);
  if (seqtrack==NULL)
    return 0;

  VALIDATE_AUTOMATIONNUM(0);
  VALIDATE_NODENUM(0);

  return SEQTRACK_AUTOMATION_get_node_x(seqtrack->seqtrackautomation, seqtrack, automationnum, nodenum);
}

float getSeqAutomationNodeY(int nodenum, int automationnum, int seqtracknum){
  struct SeqTrack *seqtrack = getSeqtrackFromNum(seqtracknum);
  if (seqtrack==NULL)
    return 0;

  VALIDATE_AUTOMATIONNUM(0);
  VALIDATE_NODENUM(0);

  return SEQTRACK_AUTOMATION_get_node_y(seqtrack->seqtrackautomation, seqtracknum, automationnum, nodenum);
}



// sequencer tempo automation
//////////////////////////////////////////

void undoSeqtempo(void){
  ADD_UNDO(SongTempoAutomation());
}

float getSeqtempoAreaX1(void){
  return SEQTEMPO_get_x1();
}
float getSeqtempoAreaY1(void){
  return SEQTEMPO_get_y1();
}
float getSeqtempoAreaX2(void){
  return SEQTEMPO_get_x2();
}
float getSeqtempoAreaY2(void){
  return SEQTEMPO_get_y2();
}
float getSeqtemponodeX(int nodenum){
  if (nodenum < 0 || nodenum >= TEMPOAUTOMATION_get_num_nodes()){
    handleError("There is no tempo node #%d", nodenum);
    return 0.0;
  }
  return TEMPOAUTOMATION_get_node_x(nodenum);
}
float getSeqtemponodeY(int nodenum){
  if (nodenum < 0 || nodenum >= TEMPOAUTOMATION_get_num_nodes()){
    handleError("There is no tempo node #%d", nodenum);
    return 0.0;
  }
  return TEMPOAUTOMATION_get_node_y(nodenum);
}
void setSeqtempoVisible(bool visible){
  SEQTEMPO_set_visible(visible);
}
bool seqtempoVisible(void){
  return SEQTEMPO_is_visible();
}

double getSeqtempoValue(int nodenum){
  if (nodenum < 0 || nodenum >= TEMPOAUTOMATION_get_num_nodes()){
    handleError("There is no tempo node #%d", nodenum);
    return 0.0;
  }
  return TEMPOAUTOMATION_get_value(nodenum);
}
double getSeqtempoAbstime(int nodenum){
  if (nodenum < 0 || nodenum >= TEMPOAUTOMATION_get_num_nodes()){
    handleError("There is no tempo node #%d", nodenum);
    return 0.0;
  }
  return TEMPOAUTOMATION_get_abstime(nodenum);
}
int getSeqtempoLogtype(int nodenum){
  if (nodenum < 0 || nodenum >= TEMPOAUTOMATION_get_num_nodes()){
    handleError("There is no tempo node #%d", nodenum);
    return 0;
  }
  return TEMPOAUTOMATION_get_logtype(nodenum);
}
int getNumSeqtemponodes(void){
  return TEMPOAUTOMATION_get_num_nodes();
}
int addSeqtemponode(double abstime, double value, int logtype){
  undoSeqtempo();
  int ret = TEMPOAUTOMATION_add_node(abstime, value, logtype);
  if (ret==-1)
    Undo_CancelLastUndo();
  return ret;
}
void deleteSeqtemponode(int nodenum){
  return TEMPOAUTOMATION_delete_node(nodenum);
}
void setCurrSeqtemponode(int nodenum){
  if (nodenum < -1 || nodenum >= TEMPOAUTOMATION_get_num_nodes()){
    handleError("There is no tempo node #%d", nodenum);
    return;
  }
  TEMPOAUTOMATION_set_curr_node(nodenum);
}
void setSeqtemponode(double abstime, double value, int logtype, int nodenum){
  if (nodenum < 0 || nodenum >= TEMPOAUTOMATION_get_num_nodes()){
    handleError("There is no tempo node #%d", nodenum);
    return;
  }
  return TEMPOAUTOMATION_set(nodenum, abstime, value, logtype);
}
void setSeqtempoLength(double end_time, bool do_shrink){
  return TEMPOAUTOMATION_set_length(end_time, do_shrink);
}
double getSeqtempoLength(void){
  return TEMPOAUTOMATION_get_length();
}
double getSeqtempoAbsabstime(double abstime){
  return TEMPOAUTOMATION_get_absabstime(abstime);
}

double getSeqtempoMaxTempo(void){
  return TEMPOAUTOMATION_get_max_tempo();
}
void setSeqtempoMaxTempo(double max_tempo){
  TEMPOAUTOMATION_set_max_tempo(max_tempo);
}


// sequencer timeline and looping
//

float getSeqtimelineAreaX1(void){
  return SEQTIMELINE_get_x1();
}
float getSeqtimelineAreaY1(void){
  return SEQTIMELINE_get_y1();
}
float getSeqtimelineAreaX2(void){
  return SEQTIMELINE_get_x2();
}
float getSeqtimelineAreaY2(void){
  return SEQTIMELINE_get_y2();
}


void setSeqlooping(bool do_loop){
  SEQUENCER_set_looping(do_loop);
}

bool isSeqlooping(void){
  return SEQUENCER_is_looping();
}

void setSeqloopingStart(int64_t start){
  SEQUENCER_set_loop_start(start);
}

int64_t getSeqloopingStart(void){
  return SEQUENCER_get_loop_start();
}

void setSeqloopingEnd(int64_t end){
  SEQUENCER_set_loop_end(end);
}

int64_t getSeqloopingEnd(void){
  return SEQUENCER_get_loop_end();
}

// seqtracks
//

float getSeqtrackX1(int seqtracknum){
  if (seqtracknum < 0 || seqtracknum >= root->song->seqtracks.num_elements){
    handleError("Sequencer track #%d does not exist", seqtracknum);
    return 0;
  }
  return SEQTRACK_get_x1(seqtracknum);
}

float getSeqtrackX2(int seqtracknum){
  if (seqtracknum < 0 || seqtracknum >= root->song->seqtracks.num_elements){
    handleError("Sequencer track #%d does not exist", seqtracknum);
    return 0;
  }
  return SEQTRACK_get_x2(seqtracknum);
}

float getSeqtrackY1(int seqtracknum){
  if (seqtracknum < 0 || seqtracknum >= root->song->seqtracks.num_elements){
    handleError("Sequencer track #%d does not exist", seqtracknum);
    return 0;
  }
  return SEQTRACK_get_y1(seqtracknum);
}

float getSeqtrackY2(int seqtracknum){
  if (seqtracknum < 0 || seqtracknum >= root->song->seqtracks.num_elements){
    handleError("Sequencer track #%d does not exist", seqtracknum);
    return 0;
  }
  return SEQTRACK_get_y2(seqtracknum);
}

int getSeqtrackFromY(int y){
  for(int seqtracknum=0;seqtracknum<getNumSeqtracks();seqtracknum++){
    float y1 = getSeqtrackY1(seqtracknum);
    float y2 = getSeqtrackY2(seqtracknum);
    //printf("y1: %f / %f,%d / %f\n", y1, tevent.x, y, y2);
    if (y>=y1 && y <= y2)
      return seqtracknum;
  }

  return -1;
}

int64_t getSeqGriddedTime(int64_t pos, int seqtracknum, const_char* type){
  if (!strcmp(type, "no"))
    return pos;
  
  else if (!strcmp(type, "line"))
    return findClosestSeqtrackLineStart(seqtracknum, pos);

  else if (!strcmp(type, "beat"))
    return findClosestSeqtrackBeatStart(seqtracknum, pos);

  else if (!strcmp(type, "bar"))
    return findClosestSeqtrackBarStart(seqtracknum, pos);

  handleError("Sequencer grid type must be either \"no\", \"line\", \"beat\", or \"bar\". (\"%s\")", type);
  return pos;
}

int64_t findClosestSeqtrackBarStart(int seqtracknum, int64_t pos){
  if (seqtracknum < 0 || seqtracknum >= root->song->seqtracks.num_elements){
    handleError("Sequencer track #%d does not exist", seqtracknum);
    return 0;
  }
  
  return SEQUENCER_find_closest_bar_start(seqtracknum, pos);
}

int64_t findClosestSeqtrackBeatStart(int seqtracknum, int64_t pos){
  if (seqtracknum < 0 || seqtracknum >= root->song->seqtracks.num_elements){
    handleError("Sequencer track #%d does not exist", seqtracknum);
    return 0;
  }
  
  return SEQUENCER_find_closest_beat_start(seqtracknum, pos);
}

int64_t findClosestSeqtrackLineStart(int seqtracknum, int64_t pos){
  if (seqtracknum < 0 || seqtracknum >= root->song->seqtracks.num_elements){
    handleError("Sequencer track #%d does not exist", seqtracknum);
    return 0;
  }
  
  return SEQUENCER_find_closest_line_start(seqtracknum, pos);
}


static const char *g_block_grid = NULL;

const_char *getSeqBlockGridType(void){
  if (g_block_grid==NULL)
    g_block_grid = SETTINGS_read_string("seq_block_grid_type", "bar");

  return g_block_grid;
}

void setSeqBlockGridType(const_char *type){
  if (!strcmp(type, "no") && !strcmp(type, "line") && !strcmp(type, "beat") && !strcmp(type, "bar")){
    handleError("Sequencer grid type must be either \"no\", \"line\", \"beat\", or \"bar\". (\"%s\")", type);
    return;
  }

  g_block_grid = talloc_strdup(type);
  SETTINGS_write_string("seq_block_grid_type", g_block_grid);
}

static const char *g_automation_grid = NULL;

const_char *getSeqAutomationGridType(void){
  if (g_automation_grid==NULL)
    g_automation_grid = SETTINGS_read_string("seq_automation_grid_type", "beat");

  return g_automation_grid;
}

void setSeqAutomationGridType(const_char *type){
  if (!strcmp(type, "no") && !strcmp(type, "line") && !strcmp(type, "beat") && !strcmp(type, "bar")){
    handleError("Sequencer grid type must be either \"no\", \"line\", \"beat\", or \"bar\". (\"%s\")", type);
    return;
  }

  g_automation_grid = talloc_strdup(type);
  SETTINGS_write_string("seq_automation_grid_type", g_automation_grid);
}

static const char *g_tempo_grid = NULL;

const_char *getSeqTempoGridType(void){
  if (g_tempo_grid==NULL)
    g_tempo_grid = SETTINGS_read_string("seq_tempo_grid_type", "beat");

  return g_tempo_grid;
}

void setSeqTempoGridType(const_char *type){
  if (!strcmp(type, "no") && !strcmp(type, "line") && !strcmp(type, "beat") && !strcmp(type, "bar")){
    handleError("Sequencer grid type must be either \"no\", \"line\", \"beat\", or \"bar\". (\"%s\")", type);
    return;
  }

  g_tempo_grid = talloc_strdup(type);
  SETTINGS_write_string("seq_tempo_grid_type", g_tempo_grid);
}


static const char *g_loop_grid = NULL;

const_char *getSeqLoopGridType(void){
  if (g_loop_grid==NULL)
    g_loop_grid = SETTINGS_read_string("seq_loop_grid_type", "beat");

  return g_loop_grid;
}

void setSeqLoopGridType(const_char *type){
  if (!strcmp(type, "no") && !strcmp(type, "line") && !strcmp(type, "beat") && !strcmp(type, "bar")){
    handleError("Sequencer grid type must be either \"no\", \"line\", \"beat\", or \"bar\". (\"%s\")", type);
    return;
  }

  g_loop_grid = talloc_strdup(type);
  SETTINGS_write_string("seq_loop_grid_type", g_loop_grid);
}



void insertSilenceToSeqtrack(int seqtracknum, int64_t pos, int64_t duration){
  struct SeqTrack *seqtrack = getSeqtrackFromNum(seqtracknum);
  if (seqtrack==NULL)
    return;

  ADD_UNDO(Sequencer());

  SEQTRACK_insert_silence(seqtrack, pos, duration);
}

int addBlockToSeqtrack(int seqtracknum, int blocknum, int64_t pos){
  struct SeqTrack *seqtrack = getSeqtrackFromNum(seqtracknum);
  if (seqtrack==NULL)
    return -1;

  struct Blocks *block = getBlockFromNum(blocknum);
  if (block==NULL)
    return -1;

  ADD_UNDO(Sequencer());

  int64_t seqtime = get_seqtime_from_abstime(seqtrack, NULL, pos);
                           
  return SEQTRACK_insert_block(seqtrack, block, seqtime);
}

int addGfxGfxBlockToSeqtrack(int seqtracknum, int blocknum, int64_t pos){
  struct SeqTrack *seqtrack = getSeqtrackFromNum(seqtracknum);
  if (seqtrack==NULL)
    return -1;

  struct Blocks *block = getBlockFromNum(blocknum);
  if (block==NULL)
    return -1;

  int64_t seqtime = get_seqtime_from_abstime(seqtrack, NULL, pos);
                           
  return SEQTRACK_insert_gfx_gfx_block(seqtrack, block, seqtime);
}

// seqblocks

int getNumSeqblocks(int seqtracknum){
  struct SeqTrack *seqtrack = getSeqtrackFromNum(seqtracknum);
  if (seqtrack==NULL)
    return 0;
  else
    return seqtrack->seqblocks.num_elements;
}

int getNumGfxGfxSeqblocks(int seqtracknum){
  struct SeqTrack *seqtrack = getSeqtrackFromNum(seqtracknum);
  if (seqtrack==NULL)
    return 0;
  else
    return seqtrack->gfx_gfx_seqblocks.num_elements;
}

int64_t getSeqblockStartTime(int seqblocknum, int seqtracknum){
  struct SeqTrack *seqtrack;
  struct SeqBlock *seqblock = getSeqblockFromNumA(seqblocknum, seqtracknum, &seqtrack);
  if (seqblock==NULL)
    return 0;

  SEQTRACK_update_all_seqblock_start_and_end_times(seqtrack);
    
  return seqblock->start_time * MIXER_get_sample_rate(); //seqblock->time;
}

int64_t getSeqblockEndTime(int seqblocknum, int seqtracknum){
  struct SeqTrack *seqtrack;
  struct SeqBlock *seqblock = getSeqblockFromNumA(seqblocknum, seqtracknum, &seqtrack);
  if (seqblock==NULL)
    return 0;

  SEQTRACK_update_all_seqblock_start_and_end_times(seqtrack);

  return seqblock->end_time * MIXER_get_sample_rate();
//return seqblock->time + getBlockSTimeLength(seqblock->block);
}

float getSeqblockX1(int seqblocknum, int seqtracknum){
  if (getSeqblockFromNum(seqblocknum, seqtracknum)==NULL)
    return 0;
  
  return SEQBLOCK_get_x1(seqblocknum, seqtracknum);
}

float getSeqblockX2(int seqblocknum, int seqtracknum){
  if (getSeqblockFromNum(seqblocknum, seqtracknum)==NULL)
    return 0;
  
  return SEQBLOCK_get_x2(seqblocknum, seqtracknum);
}

float getSeqblockY1(int seqblocknum, int seqtracknum){
  if (getSeqblockFromNum(seqblocknum, seqtracknum)==NULL)
    return 0;
  
  return SEQBLOCK_get_y1(seqblocknum, seqtracknum);
}

float getSeqblockY2(int seqblocknum, int seqtracknum){
  if (getSeqblockFromNum(seqblocknum, seqtracknum)==NULL)
    return 0;
  
  return SEQBLOCK_get_y2(seqblocknum, seqtracknum);
}

void moveSeqblock(int seqblocknum, int64_t abstime, int seqtracknum, int new_seqtracknum){
  struct SeqTrack *seqtrack;
  struct SeqBlock *seqblock = getSeqblockFromNumA(seqblocknum, seqtracknum, &seqtrack);
  if (seqblock==NULL)
    return;

  if (new_seqtracknum==-1)
    new_seqtracknum = seqtracknum;

  struct SeqTrack *new_seqtrack = getSeqtrackFromNum(new_seqtracknum);
  if (new_seqtrack==NULL)
    return;
  
  ATOMIC_SET(root->song->curr_seqtracknum, new_seqtracknum);
  
  //printf("Trying to move seqblocknum %d/%d to %d\n",seqtracknum,seqblocknum,(int)abstime);
  SEQTRACK_move_seqblock(seqtrack, seqblock, abstime);
}

void moveSeqblockGfx(int seqblocknum, int64_t abstime, int seqtracknum, int new_seqtracknum){
  struct SeqTrack *seqtrack;
  struct SeqBlock *seqblock = getSeqblockFromNumA(seqblocknum, seqtracknum, &seqtrack);
  if (seqblock==NULL)
    return;

  if (new_seqtracknum==-1)
    new_seqtracknum = seqtracknum;

  struct SeqTrack *new_seqtrack = getSeqtrackFromNum(new_seqtracknum);
  if (new_seqtrack==NULL)
    return;
  
  ATOMIC_SET(root->song->curr_seqtracknum, new_seqtracknum);
  
  //printf("Trying to move seqblocknum %d/%d to %d\n",seqtracknum,seqblocknum,(int)abstime);
  SEQTRACK_move_gfx_seqblock(seqtrack, seqblock, abstime);
}

void moveSeqblockGfxGfx(int seqblocknum, int64_t abstime, int seqtracknum, int new_seqtracknum){
  struct SeqTrack *seqtrack;
  struct SeqBlock *seqblock = getGfxGfxSeqblockFromNumA(seqblocknum, seqtracknum, &seqtrack);
  if (seqblock==NULL)
    return;

  if (new_seqtracknum==-1)
    new_seqtracknum = seqtracknum;

  struct SeqTrack *new_seqtrack = getSeqtrackFromNum(new_seqtracknum);
  if (new_seqtrack==NULL)
    return;
  
  //ATOMIC_SET(root->song->curr_seqtracknum, new_seqtracknum);
  
  //printf("Trying to move seqblocknum %d/%d to %d\n",seqtracknum,seqblocknum,(int)abstime);
  SEQTRACK_move_gfx_gfx_seqblock(seqtrack, seqblock, abstime);
}

void deleteSeqblock(int seqblocknum, int seqtracknum){
  struct SeqTrack *seqtrack;
  struct SeqBlock *seqblock = getSeqblockFromNumA(seqblocknum, seqtracknum, &seqtrack);
  if (seqblock==NULL)
    return;

  undoSequencer();
  
  SEQTRACK_delete_seqblock(seqtrack, seqblock);

  ATOMIC_SET(root->song->curr_seqtracknum, R_MAX(seqtracknum -1, 0));
  BS_UpdatePlayList();
}

void deleteGfxGfxSeqblock(int seqblocknum, int seqtracknum){
  struct SeqTrack *seqtrack;
  struct SeqBlock *seqblock = getGfxGfxSeqblockFromNumA(seqblocknum, seqtracknum, &seqtrack);
  if (seqblock==NULL)
    return;

  SEQTRACK_delete_gfx_gfx_seqblock(seqtrack, seqblock);

  SEQUENCER_update();
}

int getSeqblockBlocknum(int seqblocknum, int seqtracknum){
  struct SeqTrack *seqtrack;
  struct SeqBlock *seqblock = getSeqblockFromNumA(seqblocknum, seqtracknum, &seqtrack);
  if (seqblock==NULL)
    return 0;

  return seqblock->block->l.num;
}

/*
void selectSeqblock(int seqblocknum, int seqtracknum){
  struct SeqTrack *seqtrack;
  struct SeqBlock *seqblock = getSeqblockFromNumA(seqblocknum, seqtracknum, &seqtrack);
  if (seqblock==NULL)
    return;

  root->song->curr_seqtracknum = seqtracknum;

  selectBlock(seqblock->block->l.num, -1);
}
*/

int getNumSelectedSeqblocks(void){
  int ret = 0;
  VECTOR_FOR_EACH(struct SeqTrack *seqtrack, &root->song->seqtracks){
    VECTOR_FOR_EACH(struct SeqBlock *seqblock, &seqtrack->seqblocks){
      if (seqblock->is_selected)
        ret++;
    }END_VECTOR_FOR_EACH;
  }END_VECTOR_FOR_EACH;

  return ret;
}

void selectSeqblock(bool is_selected, int seqblocknum, int seqtracknum){
  struct SeqTrack *seqtrack;
  struct SeqBlock *seqblock = getSeqblockFromNumA(seqblocknum, seqtracknum, &seqtrack);
  if (seqblock==NULL)
    return;

  if (  seqblock->is_selected != is_selected){
    seqblock->is_selected = is_selected;
    SEQUENCER_update();
  }
}

bool isSeqblockSelected(int seqblocknum, int seqtracknum){
  struct SeqTrack *seqtrack;
  struct SeqBlock *seqblock = getSeqblockFromNumA(seqblocknum, seqtracknum, &seqtrack);
  if (seqblock==NULL)
    return false;

  return seqblock->is_selected;
}

void cutSelectedSeqblocks(void){
  evalScheme("(cut-all-selected-seqblocks)");      
}

void pasteSeqblocks(int seqtracknum, int64_t abstime){
  //printf(" pasteSeqblocks. seqtracknum: %d, abstime: %f\n", seqtracknum, (double)abstime);

  //abort();
  
  if (seqtracknum==-1)
    seqtracknum = getSeqtrackFromY(tevent.y);

  printf("seqtracknum: %d\n", seqtracknum);

  if (seqtracknum==-1)
    return;

  printf("abstime: %d\n", (int)abstime);

  if (abstime < 0){
    abstime = scale_int64(tevent.x, getSequencerX1(), getSequencerX2(), getSequencerVisibleStartTime(), getSequencerVisibleEndTime());
  }
  if (abstime < 0)
    return;

  evalScheme(talloc_format("(paste-sequencer-blocks %d " "%" PRId64 ")", seqtracknum, abstime));
}


void copySelectedSeqblocks(void){
  evalScheme("(copy-all-selected-seqblocks)");
}


void deleteSelectedSeqblocks(void){
  evalScheme("(delete-all-selected-seqblocks)");
}


