/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
if (typeof(TissueStack) == 'undefined') {
        TissueStack = {};
};

TissueStack.configuration = {
		restful_service_proxy_path : 
			{ 	value: "backend", 
				description: "restful java service proxy path (relative to the application''s web root directory)"
			}
};
TissueStack.debug = true;
TissueStack.color_maps = null;
TissueStack.sync_datasets = false;
TissueStack.overlay_datasets = false;
TissueStack.planes_swapped = false;
TissueStack.indexed_color_maps = {
	"grey" : null,
	"hot" : null,
	"spectral" : null
};
TissueStack.tasks = {};
TissueStack.cookie_lock = false;
TissueStack.lastWindowResizing = new Date().getTime();
TissueStack.transparency = 0.5;
TissueStack.reverseOverlayOrder = false;
TissueStack.swappedOverlayOrder = false;