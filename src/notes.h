#ifndef NOTES_H
#define NOTES_H

#include <stdint.h>

#include "pin_io.h"
#include "timing.h"

/*
 * The unsigned integer type used to identify
 * individual notes
 */
#define NOTE_INDEX_TYPE uint8_t

#define C4_INDEX 0u
#define D4_INDEX 1u
#define E4_INDEX 2u
#define F4_INDEX 3u
#define G4_INDEX 4u
#define A4_INDEX 5u
#define B4_INDEX 6u
#define C5_INDEX 7u

#define NOTE_INDEX_MAX_INCL C5_INDEX

/* Total number of notes used by the air piano */
#define NUM_NOTES (NOTE_INDEX_MAX_INCL + 1)

/* The canonical value to indicate an invalid note */
#define INVALID_NOTE_INDEX NUM_NOTES

/*
 * The minimum distance for a note; todo: what happens
 * if we get a reading below this value? how is it used?
 */
#define NOTE_DIST_MIN_CM  5.0f

/*
 * The maximum distance for a note; further distances
 * will be ignored
 */
#define NOTE_DIST_MAX_CM 61.0f

/*
 * This type holds the data needed to identify and
 * play notes
 */
typedef struct {
    const char *name;
    NS_TYPE     period_ns;
    NS_TYPE     duty_ns;
    PIN_ID_TYPE led_pin_id;
} NoteData;

/*
 * C4 to C5 — C major note data table;
 * period_ns and duty_ns precomputed for 50% duty cycle.
 */
static const NoteData NOTE_DATA[NUM_NOTES] = {
    { "C4",  3822192, 1911096, PIN_OUT_LED_C4 },
    { "D4",  3405482, 1702741, PIN_OUT_LED_D4 },
    { "E4",  3033816, 1516908, PIN_OUT_LED_E4 },
    { "F4",  2863708, 1431854, PIN_OUT_LED_F4 },
    { "G4",  2551020, 1275510, PIN_OUT_LED_G4 },
    { "A4",  2272727, 1136363, PIN_OUT_LED_A4 },
    { "B4",  2025237, 1012618, PIN_OUT_LED_B4 },
    { "C5",  1911096,  955548, PIN_OUT_LED_C5 },
};

/* Returns 1 if the note index is valid, 0 otherwise */
static inline uint8_t note_is_valid(
    NOTE_INDEX_TYPE index
){
    uint8_t valid = 1u;
    if(index > NOTE_INDEX_MAX_INCL){
        valid = 0u;
    }
    return valid;
}

/*
 * Given a valid note index, returns the string name
 * of that note, otherwise returns the string "invalid"
 */
static inline const char *note_get_name(
    NOTE_INDEX_TYPE index
){
    if(index > NOTE_INDEX_MAX_INCL){
        return "invalid";
    }
    return NOTE_DATA[index].name;
}

/*
 * Given a valid note index, returns the period ns
 * of that note, otherwise returns the period of the
 * first note.
 */
static inline NS_TYPE note_get_period_ns(
    NOTE_INDEX_TYPE index
){
    if(index > NOTE_INDEX_MAX_INCL){
        index = C4_INDEX;
    }
    return NOTE_DATA[index].period_ns;
}

/*
 * Given a valid note index, returns the duty ns
 * of that note, otherwise returns the duty of the
 * first note.
 */
static inline NS_TYPE note_get_duty_ns(
    NOTE_INDEX_TYPE index
){
    if(index > NOTE_INDEX_MAX_INCL){
        index = C4_INDEX;
    }
    return NOTE_DATA[index].duty_ns;
}

/*
 * Given a valid note index, returns the led pin id
 * of that note, otherwise returns the led pin id
 * first note.
 */
static inline PIN_ID_TYPE note_get_led_pin_id(
    NOTE_INDEX_TYPE index
){
    if(index > NOTE_INDEX_MAX_INCL){
        index = C4_INDEX;
    }
    return NOTE_DATA[index].led_pin_id;
}

/*
 * Maps a distance in cm to a note index
 * [0, NUM_NOTES-1].
 * Clamps distances outside
 * [NOTE_DIST_MIN_CM, NOTE_DIST_MAX_CM].
 */
static inline NOTE_INDEX_TYPE note_from_distance_cm(
    float dist_cm
){
    float range = NOTE_DIST_MAX_CM - NOTE_DIST_MIN_CM;
    float clamped = dist_cm;
    int32_t index = 0;

    if(clamped < NOTE_DIST_MIN_CM){
        clamped = NOTE_DIST_MIN_CM;
    }
    if(clamped > NOTE_DIST_MAX_CM){
        clamped = NOTE_DIST_MAX_CM;
    }

    /* calculates (dist / range) * num notes */
    index = (int32_t)
        ((clamped - NOTE_DIST_MIN_CM) * NUM_NOTES / range);

    /*
     * perform sanity check on calculated index
     * (NOTE_INDEX_MAX_INCL will not overflow
     * an int32_t)
     */
    if(index > (int32_t)NOTE_INDEX_MAX_INCL){
        index = NOTE_INDEX_MAX_INCL;
    }
    if(index < 0){
        index = 0;
    }

    /*
     * cast to smaller type, now that we aren't dealing
     * with possible large values
     */
    return (NOTE_INDEX_TYPE)index;
}

#endif /* NOTES_H */
