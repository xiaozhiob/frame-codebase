#
# This file is a part of: https://github.com/brilliantlabsAR/frame-codebase
#
# Authored by: Raj Nakarja / Brilliant Labs Ltd. (raj@brilliant.xyz)
#              Rohit Rathnam / Silicon Witchery AB (rohit@siliconwitchery.com)
#              Uma S. Gupta / Techno Exponent (umasankar@technoexponent.com)
#
# ISC Licence
#
# Copyright © 2023 Brilliant Labs Ltd.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
# REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
# INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
# OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.
#

display_driver:
	@mkdir -p simulation
	
	@iverilog -Wall \
			  -g2012 \
			  -I fpga \
			  -o simulation/display_driver_tb.out \
			  -i display_driver_tb.sv
	
	@vvp simulation/display_driver_tb.out \
		 -fst

	@gtkwave simulation/display_driver_tb.fst \
			 display_driver_tb.gtkw

sprite_engine:
	@mkdir -p simulation
	
	@iverilog -Wall \
			  -g2012 \
			  -I fpga \
			  -o simulation/sprite_engine_tb.out \
			  -i sprite_engine_tb.sv
	
	@vvp simulation/sprite_engine_tb.out \
		 -fst

	@gtkwave simulation/sprite_engine_tb.fst \
			 sprite_engine_tb.gtkw

graphics:
	@mkdir -p simulation
	
	@iverilog -Wall \
			  -g2012 \
			  -I ../../.. \
			  -o simulation/graphics_tb.out \
			  -i graphics_tb.sv
	exit
	@vvp simulation/graphics_tb.out \
		 -fst

	@gtkwave simulation/graphics_tb.fst \
			 graphics_tb.gtkw

clean:
	@rm -rf simulation
	@echo Cleaned