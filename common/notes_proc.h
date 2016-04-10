/* Copyright 2000 Kjetil S. Matheussen

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


extern LANGSPEC void SetNotePolyphonyAttributes(struct Tracks *track);
extern LANGSPEC int GetNoteSubtrack(const struct WTracks *wtrack, struct Notes *note);
extern LANGSPEC int GetNumSubtracks(const struct WTracks *wtrack);

extern LANGSPEC void StopAllNotesAtPlace(
                                         struct Blocks *block,
                                         struct Tracks *track,
                                         Place *placement
                                         );

extern LANGSPEC struct Notes *GetCurrNote(struct Tracker_Windows *window);

#define NOTE_ID_RESOLUTION 256 // i.e. 256 id's per note.
static inline int64_t NotenumId(float notenum){
  int64_t n = notenum*NOTE_ID_RESOLUTION;
  return n*NUM_PATCH_VOICES;
}

extern LANGSPEC void NOTE_init(struct Notes *note);
extern LANGSPEC struct Notes *NewNote(void);

extern LANGSPEC bool NOTES_sorted_by_pitch_questionmark(struct Notes *notes);
extern LANGSPEC struct Notes *NOTES_sort_by_pitch(struct Notes *notes);

extern LANGSPEC void NOTE_validate(const struct Blocks *block, struct Tracks *track, struct Notes *note);

extern LANGSPEC struct Notes *InsertNote(
	struct WBlocks *wblock,
	struct WTracks *wtrack,
	Place *placement,
        Place *end_placement,
	float notenum,
	int velocity,
	bool polyphonic
);

extern LANGSPEC int NOTE_get_velocity(struct Tracks *track);

extern LANGSPEC void InsertNoteCurrPos(struct Tracker_Windows *window,float notenum,bool polyphonic, float velocity);

extern LANGSPEC void LengthenNotesTo(
                     struct Blocks *block,
                     struct Tracks *track,
                     Place *placement
                     );
extern LANGSPEC void ReplaceNoteEnds(
                    struct Blocks *block,
                    struct Tracks *track,
                    Place *old_placement,
                    Place *new_placement,
                    int subtrack
                    );

extern LANGSPEC void CutNoteAt(struct Blocks *block, struct Tracks *track,struct Notes *note, Place *place);
  
extern LANGSPEC void RemoveNote(struct Blocks *block,
                struct Tracks *track,
                struct Notes *note
                );

extern LANGSPEC void RemoveNoteCurrPos(struct Tracker_Windows *window);

extern LANGSPEC struct Notes *FindPrevNoteOnSameSubTrack(struct Tracks *track, struct Notes *note);

extern LANGSPEC struct Notes *FindNoteOnSubTrack(
                                        const struct WTracks *wtrack,
                                        int subtrack,
                                        Place *placement
);

extern LANGSPEC struct Notes *FindNextNoteOnSameSubtrack(struct Notes *note);

extern LANGSPEC struct Notes *FindNextNote(
                                           struct Tracks *track,
                                           Place *placement
                                           );

extern LANGSPEC struct Notes *FindNote(
                       struct Tracks *track,
                       Place *placement
                       );

extern LANGSPEC struct Notes *FindNoteCurrPos(struct Tracker_Windows *window);

extern LANGSPEC char *notetext_from_notenum(float notenumf);
extern LANGSPEC float notenum_from_notetext(char *notetext);

extern LANGSPEC void EditNoteCurrPos(struct Tracker_Windows *window);

extern LANGSPEC void StopVelocityCurrPos(struct Tracker_Windows *window,int noend);
