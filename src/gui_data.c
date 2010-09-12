/* nekobee DSSI software synthesizer GUI
 *
 * Copyright (C) 2004 Sean Bolton and others.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "nekobee_types.h"
#include "nekobee.h"
#include "nekobee_synth.h"
#include "nekobee_voice.h"
#include "gui_main.h"
#include "gui_data.h"

/*
 * gui_data_friendly_patches
 *
 * give the new user a default set of good patches to get started with
 */
void
gui_data_friendly_patches(void)
{
    if (!(patches = (nekobee_patch_t *)malloc(128 * sizeof(nekobee_patch_t)))) {
        GDB_MESSAGE(-1, " gui_data_friendly_patches fatal: out of memory!\n");
        exit(1);
    }

//   memcpy(patches, friendly_patches, friendly_patch_count * sizeof(nekobee_patch_t));

        memcpy(&patches[0], &nekobee_init_voice, sizeof(nekobee_patch_t));

}
