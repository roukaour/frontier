# http://www.gdal.org/
# gdal-translate 10.2.1.1043901.dem -of PNM -ot UInt16 -co MAXVAL=8367 bar.ppm

import sys

def maybe(f, s):
	return f(s) if s.strip() else None

def fortran_float(s):
	return float(s.replace('D', 'E'))

def split_fixed(s, size):
	return [s[i:i+size] for i in xrange(0, len(s), size)]

def dms_to_decimal(dms):
	# dms string format: SDDDMMSS.SSSS
	# + sign is implied; no leading zeros in components
	s = -1 if dms[0] == '-' else 1
	d = float(dms[1:4])
	m = float(dms[4:6])
	s = float(dms[6:13])
	return s * (d + (m * 60 + s) / 3600)

class DEM_Profile(object):
	
	def __init__(self):
		pass
	
	def parse_b_record(self, record):
		# A two-element array containing the row and column
		# identification number of the DEM profile contained in this
		# record. See figure 2-3. The row and column numbers may range
		# from 1 to m and 1 to n. The row number is normally set to 1.
		# The column identification is the profile sequence number.
		self.row, self.column = map(int, split_fixed(record[0:12], 6))
		
		# A two-element array containing the number (m, n) of elevations
		# in the DEM profile. See figure 2-3. The first element in the
		# field corresponds to the number of rows of nodes in this
		# profile. The second element is set to 1, specifying 1 column
		# per B record.
		self.rows, self.columns = map(int, split_fixed(record[12:24], 6))
		
		# A two-element array containing the ground planimetric
		# coordinates (X_gp ,Y_gp) of the first elevation in the
		# profile. See figure 2-3.
		self.first_elevation_ground_planimetric_coords = tuple(map(fortran_float, split_fixed(record[24:72], 24)))
		
		# The values are in the units of measure given by data element
		# 9, logical record type A.
		self.local_datum_elevation = fortran_float(record[72:96])
		
		# A two-element array of minimum and maximum elevations for the
		# profile. The values are in the units of measure given by data
		# element 9 in logical record type A and are the algebraic
		# result of the method outlined in data element 6 of this
		# record.
		self.min_elevation, self.max_elevation = map(fortran_float, split_fixed(record[96:144], 24))
		
		# An m,n array of elevations for the profile. Elevations are
		# expressed in units of resolution. A maximum of six characters
		# are allowed for each integer elevation value. See data element
		# 15 in appendix 2-A. A value in this array would be multiplied
		# by the "z spatial resolution (data element 15, record type A)"
		# and added to the "Elevation of local datum for the profile
		# (data element 4, record type B)" to obtain the elevation for
		# the point. The planimetric ground coordinates of point X_gp,
		# Y_gp are computed according to the formulas in figure 2-3.
		self.elevations = [[None] * self.rows for i in xrange(self.columns)]
		row = 0
		col = 0
		pos = 145
		while pos + 6 <= DEM.BLOCK_SIZE:
			self.elevations[col][row] = int(record[pos:pos+6])
			col += 1
			if col == self.columns:
				col = 0
				row += 1
			if row == self.rows:
				return None
			pos += 6
		return (row, col)
	
	def parse_continued_b_record(self, record, next_coords):
		row, col = next_coords
		pos = 0
		while pos + 6 <= DEM.BLOCK_SIZE:
			self.elevations[col][row] = int(record[pos:pos+6])
			col += 1
			if col == self.columns:
				col = 0
				row += 1
			if row == self.rows:
				return None
			pos += 6
		return (row, col)

class DEM(object):
	
	BLOCK_SIZE = 1024
	
	def __init__(self):
		pass
	
	def parse(self, path):
		# Open DEM file
		f = open(path, 'rb')
		# Parse A record
		a_record = f.read(DEM.BLOCK_SIZE)
		self.parse_a_record(a_record)
		# Parse B records
		self.dem_profiles = {}
		for i in xrange(self.columns):
			profile = DEM_Profile()
			b_record = f.read(DEM.BLOCK_SIZE)
			next_coords = profile.parse_b_record(b_record)
			while next_coords:
				continued_b_record = f.read(DEM.BLOCK_SIZE)
				next_coords = profile.parse_continued_b_record(continued_b_record, next_coords)
			self.dem_profiles[(profile.row, profile.column)] = profile
		# Parse C record
		c_record = f.read(DEM.BLOCK_SIZE)
		self.parse_c_record(c_record)
		# Close DEM file
		f.close()
	
	def parse_a_record(self, record):
		# The authorized digital cell name followed by a comma, space,
		# and the two-character State designator(s) separated by
		# hyphens. Abbreviations for other countries, such as Canada and
		# Mexico, shall not be represented in the DEM header.
		self.file_name = record[0:40].strip()
		
		# Free format descriptor field, contains useful information
		# related to digital process such as digitizing instrument,
		# photo codes, slot widths, etc.
		self.file_description = record[40:80].strip()
		
		# filler = record[80:109]
		
		# SE geographic quadrangle corner ordered as:
		# x = Longitude = SDDDMMSS.SSSS
		# y = Latitude = SDDDMMSS.SSSS
		# (neg sign (S) right justified, no leading zeroes, plus sign
		# (S) implied)
		self.se_corner_lat_lon = tuple(map(dms_to_decimal, split_fixed(record[109:135], 13)))
		
		# 1=Autocorrelation RESAMPLE Simple bilinear
		# 2=Manual profile GRIDEM Simple bilinear
		# 3=DLG/hypsography CTOG 8-direction linear
		# 4=Interpolation from photogrammetric system contours DCASS
		#   4-direction linear
		# 5=DLG/hypsography LINETRACE, LT4X Complex linear
		# 6=DLG/hypsography CPS-3, ANUDEM, GRASS Complex polynomial
		# 7=Electronic imaging (non-photogrametric), active or
		#   passive, sensor systems
		self.process_code = int(record[135:136])
		
		# filler = record[136:137]
		
		# This code is specific to 30-minute DEM's. Identifies
		# 1:100,000-scale sections.
		self.sectional_indicator = record[137:140].strip()
		
		# Free format Mapping Origin Code. Example: MAC, WMC, MCMC,
		# RMMC, FS, BLM, CONT (contractor), XX (state postal code).
		self.origin_code = record[140:144].strip()
		
		# 1=DEM-1
		# 2=DEM-2
		# 3=DEM-3
		# 4=DEM-4
		self.dem_level_code = int(record[144:150])
		
		# 1=regular
		# 2=random, reserved for future use
		self.elevation_pattern_code = int(record[150:156])
		
		# 0=Geographic
		# 1=UTM
		# 2=State plane
		# For codes 3-20, see Appendix 2-G. Code 0 represents the
		# geographic (latitude/longitude) system for 30-minute, 1-degree
		# and Alaska DEM's. Code 1 represents the current use of the UTM
		# coordinate system for 7.5-minute DEM's
		self.ground_planimetric_reference_system_code = int(record[156:162])
		
		# Codes for State plane and UTM coordinate zones are given in
		# appendixes 2-E and 2-F for 7.5-minute DEM's. Code is set to
		# zero if element 5 is also set to zero, defining data as
		# geographic.
		self.ground_planimetric_reference_system_zone_code = int(record[162:168])
		
		# Definition of parameters for various projections is given in
		# Appendix F. All 15 fields of this element are set to zero and
		# should be ignored when geographic, UTM, or State plane
		# coordinates are coded in data element 5.
		
		# Definition of parameters for various projections is given in
		# Appendix F. All 15 fields of this element are set to zero and
		# should be ignored when geographic, UTM, or State plane
		# coordinates are coded in data element 5.
		self.map_projection_params = tuple(map(fortran_float, split_fixed(record[168:528], 24)))
		
		# 0=radians
		# 1=feet
		# 2=meters
		# 3=arc-seconds
		# Normally set to code 2 for 7.5-minute DEM's. Always set to
		# code 3 for 30-minute, 1-degree, and Alaska DEMs.
		self.ground_planimetric_coord_unit_code = int(record[528:534])
		
		# 1=feet
		# 2=meters
		# Normally code 2, meters, for 7.5-minute, 30-minute, 1-degree,
		# and Alaska DEM's.
		self.elevation_coord_unit_code = int(record[534:540])
		
		# Set to n=4.
		self.dem_coverage_polygon_sides = int(record[540:546])
		
		# The coordinates of the quadrangle corners are ordered in a
		# clockwise direction beginning with the southwest corner. The
		# array is stored as as pairs of eastings and northings.
		self.dem_quadrangle_boundary_ground_coords = tuple(map(tuple, split_fixed(map(fortran_float, split_fixed(record[546:738], 24)), 2)))
		
		# The values are in the unit of measure given by data element 9
		# in this record and are the algebraic result of the method
		# outlined in data element 6, logical record B.
		self.dem_min_elevation, self.dem_max_elevation = map(fortran_float, split_fixed(record[738:786], 24))
		# Counterclockwise angle (in radians) from the primary axis of
		# ground planimetric reference to the primary axis of the DEM
		# local reference system. See figure 2-3. Set to zero to align
		# with the coordinate system specified in element 5.
		self.ground_planimetric_reference_system_angle = fortran_float(record[786:810])
		
		# 0=unknown accuracy
		# 1=accuracy information is given in logical record type C
		self.elevation_accuracy_code = int(record[810:816])
		
		# A three-element array of DEM spatial resolution for x, y, z.
		# Values are expressed in units of resolution. The units of
		# measure are consistent with those indicated by data elements 8
		# and 9 in this record.
		# Only integer values are permitted for the x and y resolutions.
		# For all USGS DEMs except the 1-degree DEM, z resolutions of 1
		# decimal place for feet and 2 decimal places for meters are
		# permitted.
		self.dem_spatial_resolution = tuple(map(fortran_float, split_fixed(record[816:852], 12)))
		
		# When the row value m is set to 1 the n value describes the
		# number of columns in the DEM file.
		self.rows, self.columns = map(int, split_fixed(record[852:864], 6))
		
		# Present only if two or more primary intervals exist (level 2
		# DEM's only).
		self.max_primary_contour_interval = maybe(int, record[864:869])
		
		# Corresponds to the units of the map largest primary contour
		# interval (level 2 DEM's only).
		# 0=N.A.
		# 1=feet
		# 2=meters
		self.source_contour_interval_units = maybe(int, record[869:870])
		
		# Corresponds to the units of the map smallest primary contour
		# interval (level 2 DEM's only).
		# 1=feet
		# 2=meters
		self.min_primary_contour_interval = maybe(int, record[870:875])
		
		# "YYYY" 4 character year, e.g. 1975, 1997, 2001, etc.
		# Synonymous with the original compilation data and/or the date
		# of the photography.
		self.data_source_data = int(record[876:880])
		
		# "YYYY" 4 character year. Synonymous with the date of
		# completion and/or the date of revision.
		self.data_revision_data = int(record[880:884])
		
		# "I" Indicates all processes of part 3, Quality Control have
		# been performed.
		self.inspection_flag = record[884:885]
		
		# 0=No validation performed.
		# 1=RMSE computed from test points (record C added), no
		#   quantitative test, no interactive DEM editing or review.
		# 2=Batch process water body edit and RMSE computed from test
		#   points.
		# 3=Review and edit, including water edit. No RMSE computed from
		#   test points.
		# 4=Level 1 DEM's reviewed and edited. Includes water body
		#   editing. RMSE computed from test points.
		# 5=Level 2 and 3 DEM's reviewed and edited. Includes water body
		#   editing and verification or vertical integration of
		#   planimetric categories (other than hypsography or
		#   hydrography if authorized). RMSE computed from test points.
		self.data_validation_flag = int(record[885:886])
		
		# 0=none
		# 1=suspect areas
		# 2=void areas
		# 3=suspect and void areas
		self.suspect_and_void_area_flag = int(record[886:888])
		
		# 1=local mean sea level
		# 2=National Geodetic Vertical Datum 1929 (NGVD 29)
		# 3=North American Vertical Datum 1988 (NAVD 88)
		# (note: see appendix 2-H for datum information)
		self.vertical_datum = int(record[888:890])
		
		# 1=North American Datum 1927 (NAD 27)
		# 2=World Geodetic System 1972 (WGS 72)
		# 3=WGS 84
		# 4=NAD 83
		# 5=Old Hawaii Datum
		# 6=Puerto Rico Datum
		# (note: see appendix 2-H for datum information)
		self.horizontal_datum = int(record[890:892])
		
		# 01-99
		# Primarily a DMA specific field. (For USGS use, set to 01)
		self.data_edition = int(record[892:896])
		
		# If element 25 indicates a void, this field (right justified)
		# contains the percentage of nodes in the file set to void
		# (-32,767).
		self.percent_void = maybe(int, record[896:900])
		
		# Edge match status flag. Ordered West, North, East, and South.
		# See section 2.2.4 for valid flags and explanation of codes.
		self.edge_match_flag = tuple(map(int, split_fixed(record[900:908], 2)))
		
		# Value is in the form of SFFF.DD. Value is the average shift
		# value for the four quadrangle corners obtained from program
		# VERTCON. Always add this value to convert to NAVD88.
		self.vertical_datum_shift = fortran_float(record[908:915])
	
	def parse_c_record(self, record):
		# Code indicating availability of statistics in data element 2.
		# 1=available
		# 0=unavailable
		self.has_absolute_datum_rmse = maybe(int, record[0:6])
		
		# RMSE of file's datum relative to absolute datum (x, y, z).
		# RMSE integer values are in the same unit of measure given by
		# data elements 8 and 9 of logical record type A.
		self.absolute_datum_rmse = tuple(map(int, split_fixed(record[6:24], 6))) or None
		
		# Sample size on which statistics in data element 2 are based.
		# If 0, then accuracy will be assumed to be estimated rather
		# than computed.
		self.absolute_datum_rmse_sample_size = maybe(int, record[24:30])
		
		# Code indicating availability of statistics in data element 5.
		# 1=available
		# 0=unavailable
		self.has_relative_datum_rmse = maybe(int, record[30:36])
		
		# RMSE of DEM data relative to file's datum (x, y, z).
		# RMSE integer values are in the same unit of measure given by
		# data elements 8 and 9 of logical record type A.
		self.relative_datum_rmse = tuple(map(int, split_fixed(record[36:54], 6))) or None
		
		# Sample size on which statistics in data element 5 are based.
		# If 0, then accuracy will be assumed to be estimated rather
		# than computed.
		self.relative_datum_rmse_sample_size = maybe(int, record[54:60])

def main():
	# Parse DEM
	dem_path = r'E:\Desktop\cse528-terrain\10.2.1.1043901.dem'
	dem = DEM()
	dem.parse(dem_path)
	# Print DEM data
	w = max(len(dem.dem_profiles[c].elevations[0]) for c in dem.dem_profiles)
	h = dem.columns
	sys.stderr.write('%d by %d\n' % (w, h))
	min_el = min(min(dem.dem_profiles[c].elevations[0]) for c in dem.dem_profiles)
	max_el = max(max(dem.dem_profiles[c].elevations[0]) for c in dem.dem_profiles)
	el_range = max_el - min_el
	sys.stderr.write('%d to %d is %d\n' % (min_el, max_el, el_range))
	sys.stdout.write('P3 %d %d %d\n' % (w, h, el_range))
	for r in xrange(1, h+1):
		es = dem.dem_profiles[(1, r)].elevations[0]
		if r < h / 2:
			for c in xrange(w - len(es)):
				sys.stdout.write('0 0 0 ')
		for el in es:
			#el = int(255 * float(el - min_el) / el_range)
			el = el - min_el
			sys.stdout.write('%d %d %d ' % (el, el, el))
		sys.stderr.write('row %d has %d; needs %d more\n' % (r, len(es), w - len(es)))
		if r >= h / 2:
			for c in xrange(w - len(es)):
				sys.stdout.write('0 0 0 ')
		sys.stdout.write('\n')

if __name__ == '__main__':
	main()
