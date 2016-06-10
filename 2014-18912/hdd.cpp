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
	_head_pos = 0;
	if(outer < inner){
		cout << "invalid surface shape" << endl;
	}
	if(_rpm == 0){
		cout << "invalid rpm : rpm equals to zero" << endl;
	}
	tracks_per_sf = tracks_per_surface;
	uint64 total_sectors = 0;
	for(uint32 track=0; track<tracks_per_surface;track++){
		total_sectors += num_of_sector(track) * _surfaces;
	}
  //
  // print info
  //
  cout.precision(3);
  double cap = (double)total_sectors * _sector_size / 1000000000.0;
  cout << "HDD: " << endl
       << "  surfaces:                  " << _surfaces << endl
       << "  tracks/surface:            " << tracks_per_surface << endl
       << "  sect on innermost track:   " << sectors_innermost_track << endl
       << "  sect on outermost track:   " << sectors_outermost_track << endl
       << "  rpm:                       " << rpm << endl
       << "  sector size:               " << _sector_size << endl
	   << "  number of sectors total:   " << total_sectors << endl
	   << "  capacity (GB):             " << dec << fixed << cap << endl
       << endl;
}

HDD::~HDD(void)
{
  // TODO
	if(_verbose)
		cout << "HDD removed" << endl;
}

uint64 min(uint64 a,uint64 b){
	if(a<b)return a;
	return b;
}

double HDD::read(double ts, uint64 address, uint64 size)
{
  // TODO
	HDD_Position pos;
	uint64 num_sector = (size+_sector_size-1)/_sector_size;
	// how many sectors do we need to read
	double delay_t = 0;
	// add ts at last because of error
	while(num_sector > 0){
		if(!decode(address, &pos)){
			cout << "invalid parameter" << endl;
			return ts + delay_t;
		}
		delay_t += seek_time(_head_pos, pos.track);
		_head_pos = pos.track;
		// _head_pos move to pos.track
		delay_t += wait_time();
		uint64 access_sectors = min(num_sector, pos.max_access);
		// read access_sectors at once
		num_sector -= access_sectors;
		delay_t += read_time(access_sectors);

		// get next address
		address += (uint64)access_sectors * _sector_size;
	}
	return ts + delay_t;
}

double HDD::write(double ts, uint64 address, uint64 size)
{
  // TODO
	HDD_Position pos;
	uint64 num_sector = (size+_sector_size-1)/_sector_size;
	// how many sectors do we need to write
	double delay_t = 0;
	// add ts at last because of error
	while(num_sector > 0){
		if(!decode(address, &pos)){
			cout << "invalid parameter" << endl;
			return ts + delay_t;
		}
		delay_t += seek_time(_head_pos, pos.track);
		_head_pos = pos.track;
		// _head_pos move to pos.track
		delay_t += wait_time();
		uint64 access_sectors = min(num_sector, pos.max_access);
		// write access_sectors at once
		num_sector -= access_sectors;
		delay_t += write_time(access_sectors);

		// get next address
		address += (uint64)access_sectors * _sector_size;
	}
	return ts + delay_t;
}

double HDD::seek_time(uint32 from_track, uint32 to_track)
{
  // TODO
	// if we don't need to move head
	if(from_track == to_track) return 0.0;

	uint32 diff;
	if(from_track < to_track){
		diff = to_track - from_track;
	}
	else{
		diff = from_track - to_track;
	}
	// diff = track distance
	return _seek_overhead + _seek_per_track * diff;
}

double HDD::wait_time(void)
{
  // TODO
	//average wait_time
	return 30.0 / _rpm;
}

double HDD::read_time(uint64 sectors)
{
  // TODO
	//get read time on this track
	return 60.0 / _rpm * sectors / (num_of_sector(_head_pos));
}

double HDD::write_time(uint64 sectors)
{
  // TODO
	//get write time on this track
	return 60.0 / _rpm * sectors / num_of_sector(_head_pos);
}

bool HDD::decode(uint64 address, HDD_Position *pos)
{
  // TODO
	address /= _sector_size;
	// now, address = sector address

	// address is sector-major
	for(uint32 track = 0; track<tracks_per_sf;track++){

		uint64 sectors = (uint64) _surfaces * num_of_sector(track);
		// num of sectors in this track
		if(address < sectors){ // position in this track
			pos -> surface = address / num_of_sector(track);
			// which surface?
			pos -> track = track;
			// surface, track determined
			address %= num_of_sector(track);
			pos -> sector = address;
			// get sector index
			pos -> max_access = (num_of_sector(track) - pos->sector) +
					(_surfaces - pos->surface - 1) * num_of_sector(track);
			// how many sectors until end of this track

			return true;
			// decode success
		}
		else{ // otherwise, go to next track
			address -= sectors;
		}
	}
	return false;
}


uint32 HDD::num_of_sector(uint32 track_index){
	// val : internal division with inner, outer
	uint64 val = (uint64)inner*(tracks_per_sf-1-track_index)
				+ (uint64)outer*track_index;
	val /= (tracks_per_sf - 1);

	return (uint32)val;
}




