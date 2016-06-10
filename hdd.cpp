//------------------------------------------------------------------------------
/// @brief rotating disk-based storage devices (HDD)
/// @author Bernhard Egger <bernhard@csap.snu.ac.kr>
/// @section changelog Change Log
/// 2016/05/22 Bernhard Egger created
///
/// @section license_section License
/// Copyright (c) 2016, Bernhard Egger
/// All rights reserved.
///
/// Redistribution and use in source and binary forms,  with or without modifi-
/// cation, are permitted provided that the following conditions are met:
///
/// - Redistributions of source code must retain the above copyright notice,
///   this list of conditions and the following disclaimer.
/// - Redistributions in binary form must reproduce the above copyright notice,
///   this list of conditions and the following disclaimer in the documentation
///   and/or other materials provided with the distribution.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY  AND FITNESS FOR A PARTICULAR PURPOSE
/// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER  OR CONTRIBUTORS BE
/// LIABLE FOR ANY DIRECT,  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSE-
/// QUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF  SUBSTITUTE
/// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
/// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT
/// LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY
/// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
/// DAMAGE.
//------------------------------------------------------------------------------

#include <cassert>
#include <limits>
#include <cmath>
#include <cstdlib>

#include <iostream>
#include <iomanip>

#include "hdd.h"
using namespace std;

//------------------------------------------------------------------------------
// HDD
//
HDD::HDD(uint32 surfaces, uint32 tracks_per_surface,
         uint32 sectors_innermost_track, uint32 sectors_outermost_track,
         uint32 rpm, uint32 sector_size,
         double seek_overhead, double seek_per_track,
         bool verbose)
  : _surfaces(surfaces), _rpm(rpm), _sector_size(sector_size),
    _seek_overhead(seek_overhead), _seek_per_track(seek_per_track),
    _verbose(verbose)
{
  // TODO
	inner = sectors_innermost_track;
	outer = sectors_outermost_track;
	tracks_per_sf = tracks_per_surface;
  //
  // print info
  //
  cout.precision(3);
  cout << "HDD: " << endl
       << "  surfaces:                  " << _surfaces << endl
       << "  tracks/surface:            " << tracks_per_surface << endl
       << "  sect on innermost track:   " << sectors_innermost_track << endl
       << "  sect on outermost track:   " << sectors_outermost_track << endl
       << "  rpm:                       " << rpm << endl
       << "  sector size:               " << _sector_size << endl
       << endl;
}

HDD::~HDD(void)
{
  // TODO
	cout << "HDD removed" << endl;
}

double HDD::read(double ts, uint64 address, uint64 size)
{
  // TODO
	HDD_Position pos;
	decode(address, &pos);
	double end_time = ts + seek_time(_head_pos, pos.track)
						+ wait_time() + read_time(num_of_sector(pos.track));
	_head_pos = pos.track;
	return end_time;
}

double HDD::write(double ts, uint64 address, uint64 size)
{
  // TODO
	HDD_Position pos;
	decode(address, &pos);
	double end_time = ts + seek_time(_head_pos, pos.track)
						+ wait_time() + write_time(num_of_sector(pos.track));
	_head_pos = pos.track;
	return end_time;
}

double HDD::seek_time(uint32 from_track, uint32 to_track)
{
  // TODO
	uint32 diff;
	if(from_track < to_track){
		diff = to_track - from_track;
	}
	else{
		diff = from_track - to_track;
	}
  return _seek_overhead + _seek_per_track * diff;
}

double HDD::wait_time(void)
{
  // TODO
  return 0.0;
}

double HDD::read_time(uint64 sectors)
{
  // TODO
	return 60.0 / _rpm / sectors;
}

double HDD::write_time(uint64 sectors)
{
  // TODO
	return 60.0 / _rpm / sectors;
}

bool HDD::decode(uint64 address, HDD_Position *pos)
{
  // TODO
	for(uint32 track = 0; track<tracks_per_sf;track++){
		uint64 bytes = (uint64)_sector_size * _surfaces * num_of_sector(track);
		if(address < bytes){
			uint64 bytes_per_sf = (uint64)_sector_size * num_of_sector(track);
			pos -> surface = address / bytes_per_sf;
			pos -> track = track;
			address %= bytes_per_sf;
			pos -> sector = address / _sector_size;
			// what is max_access?

			return true;
		}
		else{
				address -= bytes;
		}
	}
  return false;
}


uint32 HDD::num_of_sector(uint32 track_index){
	uint64 val = (uint64)inner*(tracks_per_sf-1-track_index)
				+ (uint64)outer*track_index;
	val /= (tracks_per_sf - 1);

	return (uint32)val;
}














