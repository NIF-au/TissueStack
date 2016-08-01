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
TissueStack.Measurements = function(canvas) {
    this.canvas = canvas;
    this.measurements = [];
};

TissueStack.Measurements.prototype = {
    canvas : null,
    measurements : null,
    checkMeasurements : function(point) {
        if (typeof point !== 'object' || point === null ||
            typeof point.x !== 'number' || typeof point.y !== 'number' ||
            typeof point.z !== 'number')
            return false;

        for (var p in this.measurements) // plane check
            if (this.measurements[p].z !== point.z) {
                this.resetMeasurements();
                break;
            }
        return true;
	}, addMeasure : function(point, isPixelMeasure) {
        if (!this.checkMeasurements(point)) return;

        if (typeof isPixelMeasure !== 'boolean')
            isPixelMeasure = true;

        // note: store them as pixels and convert only when measuring
        var realWorldCoords = point;
        if (isPixelMeasure) {
            realWorldCoords =
                this.canvas.data_extent.getWorldCoordinatesForPixel(point);
            if (typeof realWorldCoords === 'object')
                realWorldCoords.z = point.z;
        }
        if (this.checkMeasurements(realWorldCoords)) {
            this.measurements.push(realWorldCoords);
            this.canvas.drawMe();
        }
    }, measureDistance : function() {
        if (this.measurements.length <= 1) return 0.0;

        var distance = 0.0;

        for (var i=0,j=1;j<this.measurements.length;i++,j++)
            distance += Math.sqrt(
                Math.pow(this.measurements[j].x - this.measurements[i].x, 2) +
                Math.pow(this.measurements[j].y - this.measurements[i].y, 2));

        return distance;
    }, measureArea : function() {
        if (this.measurements.length <= 2) return 0.0;

        var area = 0.0;

        var closedMeasurements =
            this.measurements.concat(this.measurements[0]);
        for (var i=0,j=1;j<closedMeasurements.length;i++,j++)
            area +=
                closedMeasurements[i].x * closedMeasurements[j].y -
                closedMeasurements[i].y * closedMeasurements[j].x;

        return Math.abs(area) / 2;
    }, resetMeasurements : function() {
        this.measurements = [];
        this.canvas.drawMe();
    }, drawMeasuring : function() {
        if (this.measurements.length === 0) return;

        var ctx = this.canvas.getCanvasContext();
        if (TissueStack.overlay_datasets && this.canvas.underlying_canvas)
            ctx.globalAlpha = 1;

        ctx.strokeStyle="rgba(255,255,0,1)";
        ctx.fillStyle="rgba(255,255,0,1)";

        var numOfPoints = this.measurements.length;
        var firstCoords = null;
        var lastCoords = null;
        for (var i=0;i<numOfPoints;i++) {
            var pixelCoords =
                this.canvas.data_extent.getPixelForWorldCoordinates(
                    this.measurements[i]);
            // correct by image offset
            pixelCoords.x += this.canvas.upper_left_x;
            pixelCoords.y += (this.canvas.dim_y - this.canvas.upper_left_y);

            if (i === 0)
                firstCoords = pixelCoords;
            else {
                ctx.beginPath();
                ctx.moveTo(lastCoords.x, lastCoords.y);
                ctx.lineTo(pixelCoords.x, pixelCoords.y);
                ctx.stroke();
                ctx.closePath();
            }
            ctx.beginPath();
            ctx.arc(pixelCoords.x, pixelCoords.y, 1.5, 0, 2* Math.PI);
            ctx.fill();
            ctx.closePath();
            lastCoords = pixelCoords;
            if (i > 1 && i === numOfPoints-1) {
                ctx.beginPath();
                ctx.save();
                ctx.setLineDash([1, 1]);
                ctx.strokeStyle="rgba(0,255,0,1)";
                ctx.moveTo(pixelCoords.x, pixelCoords.y);
                ctx.lineTo(firstCoords.x, firstCoords.y);
                ctx.stroke();
                ctx.restore();
                ctx.closePath();
            }
        }
        if (TissueStack.overlay_datasets && this.canvas.underlying_canvas)
            ctx.globalAlpha = TissueStack.transparency;
    },getNumberOfMeasurements : function() {
        return this.measurements.length;
    }, dispose : function() {
        $("#" + this.canvas.dataset_id + "_main_view_canvas").off("contextmenu");
        this.measurements = [];
        this.canvas = null;

        var myMeasuringContext = $("#measuringContextMenu");
        if (!myMeasuringContext || myMeasuringContext.length === 0) return;
        myMeasuringContext.off("mouseover mouseout");
    }
};
