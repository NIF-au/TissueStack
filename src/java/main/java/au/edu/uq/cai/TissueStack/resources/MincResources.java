package au.edu.uq.cai.TissueStack.resources;

import java.io.File;

import javax.ws.rs.Path;
import javax.ws.rs.QueryParam;

import net.sf.json.JSONArray;
import net.sf.json.JSONException;

import au.edu.uq.cai.TissueStack.dataobjects.DataSet;
import au.edu.uq.cai.TissueStack.dataobjects.MincInfo;
import au.edu.uq.cai.TissueStack.dataobjects.NoResults;
import au.edu.uq.cai.TissueStack.dataobjects.Response;
import au.edu.uq.cai.TissueStack.dataprovider.DataSetDataProvider;
import au.edu.uq.cai.TissueStack.jni.TissueStack;
import au.edu.uq.cai.TissueStack.rest.AbstractRestfulMetaInformation;
import au.edu.uq.cai.TissueStack.rest.Description;
import au.edu.uq.cai.TissueStack.rest.JSONBodyWriter;
import au.edu.uq.cai.TissueStack.utils.StringUtils;

@Path("/minc")
@Description("MINC Resources")
public final class MincResources extends AbstractRestfulMetaInformation {

	@Path("/info")
	@Description("Test Case")
	public RestfulResource getInfo(
			@Description("Paramter 'file': specify the file to query")
			@QueryParam("file")
			String mincFile){
		if (mincFile == null) {
			return new RestfulResource(new Response(new NoResults()));
		} else if (!new File(mincFile).exists()) {
			throw new IllegalArgumentException("File " + mincFile + " could not be found");
		}
		MincInfo results = new TissueStack().getMincInfo(mincFile);
		return new RestfulResource(new Response(results));
	}

	
	//jstring base_dir, jintArray arr_dimensions, jdouble zoom_factor, jboolean preview
	@Path("/tile")
	@Description("Tiles a given data set")
	public RestfulResource tileDataSet(
			@Description("Parameter 'base_directory': base directory for the tiles. default: /tmp/tiles")
			@QueryParam("tile_dir")
			String baseDirectory,
			@Description("Parameter 'dataset_id': data set id of the data set to be tiled (Either 'file' or 'dataset_id' have to be given!!!)")
			@QueryParam("dataset_id")
			String dataSetId,
			@Description("Parameter 'file': file name of the data to be tiled (Either 'file' or 'dataset_id' have to be given!!!)")
			@QueryParam("file")
			String mincFile,
			@Description("Optional: dimensions to be tiled (comma separated) default: -1")
			@QueryParam("dimensions")
			String dimensions,
			@Description("Optional: zoom level. default: 0")
			@QueryParam("zoom")
			String zoom,
			@Description("Optional: tile size. default: 256")
			@QueryParam("tile_size")
			String tileSize,
			@Description("Optional: tile as low res preview ('true' or 'false'), default: false")
			@QueryParam("preview")
			String preview,
			@Description("Optional: store a non-existant data set ('true' or 'false'), default: false")
			@QueryParam("store_data_set")
			String storeDataSet,
			@Description("Optional: the image type, default: PNG")
			@QueryParam("image_type")
			String imageType){			
		// check tile directory
		if (baseDirectory == null || baseDirectory.trim().isEmpty()) {
			baseDirectory = "/tmp/tiles";
		}
		File tileDir = new File(baseDirectory);
		if (!tileDir.exists()) {
			// try to create non existing directories
			if (!tileDir.mkdirs()) {
				throw new IllegalArgumentException("Could not create tile directory: " + tileDir.getAbsolutePath());
			}
		}

		DataSet dataSet = null;
		MincInfo associatedMincInfo = null;

		boolean missingSource = true;
		// look for data set id
		try {
			dataSet = DataSetDataProvider.queryDataSetById(Long.parseLong(dataSetId));
			if (dataSet != null) {
				missingSource = false;
				mincFile = dataSet.getFilename();
			}
		} catch(Exception idNotThereOrNotNUmeric) {
			// we ignore that
		}
		// look for file
		if (dataSet == null && mincFile != null && new File(mincFile).exists()) {
			missingSource = false;
			try {
				dataSet = DataSetDataProvider.queryDataSetByFileName(mincFile);
			} catch (Exception e) {
				// we can ignore that, we have to rely on the file solely
			}
		}

		// check whether we have either a file or valid id as input
		if (missingSource) {
			throw new IllegalArgumentException(
					"Not a valid source. You have to hand in either an existing data set id or a valid minc file location!");
		}

		boolean storeDataSetAsBoolean = false; 
		try {
			storeDataSetAsBoolean = Boolean.parseBoolean(storeDataSet);
		} catch (Exception e) {
			// fall back onto default
		}
		
		// evaluate image type
		if (imageType == null || imageType.trim().isEmpty()) {
			imageType = "PNG";
		}
		
		final TissueStack jniTissueStack = new TissueStack();
		associatedMincInfo = jniTissueStack.getMincInfo(mincFile);
		// if we didn't find a data set in the db, query the minc file and populate a data set
		if (dataSet == null) {
			dataSet = DataSet.fromMincInfo(associatedMincInfo);
			if (storeDataSetAsBoolean) DataSetDataProvider.insertNewDataSets(dataSet);
		}
		// throw an error if we could not create a data set out of the given minc file
		if (dataSet == null || associatedMincInfo == null) {
			throw new RuntimeException("Could not create data set from given minc file: " + mincFile + ". Check if valid...");
		}
		
		// augment the path by the data set id
		tileDir = new File(tileDir, String.valueOf(dataSet.getId()));
		
		// check rest of params now
		final String dims[] = StringUtils.convertCommaSeparatedQueryParamsIntoStringArray(dimensions, true);
		int dimensionsArray[] = new int[] {-1,-1,-1,-1,-1,-1};
		// fill array with given values
		if (dims != null) {
			if (dims.length > 6) {
				throw new RuntimeException("Dimension parameter can at most have 6 comma separated values (== 3 dimensions)");
			}

			// convert strings and populate dimensionsArray
			int counter = 0;
			for (String d : dims) {
				try {
					dimensionsArray[counter] = Integer.parseInt(d);
				} catch (Exception e) {
					// propagate
					throw new RuntimeException("Dimension parameter has to be numeric (only exception: comma)");
				}
				counter++;
			}
			
			// double check bounds
			for (int i = 0;i<dimensionsArray.length;i++) {
				int arrayValue = dimensionsArray[i];
				if (arrayValue < -1 || ((i / 2) < associatedMincInfo.getSizes().length && arrayValue > associatedMincInfo.getSizes()[i / 2] - 1)) {
					throw new RuntimeException("A given dimension value is outside the min/max range");
				}
				// check if start is not exceeding end
				if ((i % 2) == 0 && dimensionsArray[i+1] != 0 && arrayValue > dimensionsArray[i+1]) {
					throw new RuntimeException("The start for a given dimension value is larger than the end value for that dimension");
				}
			}
		}
		
		int zoomLevel = 0;
		try {
			zoomLevel = Integer.parseInt(zoom);
		} catch (Exception e) {
			// fall back onto default
		}
		
		double zoomFactor = 1;
		try {
			final String zoomLevelJson = dataSet.getPlanes().get(0).getZoomLevels();
			final JSONArray zoomLevels = JSONArray.fromObject(zoomLevelJson, JSONBodyWriter.getJsonConfig());
			if (zoomLevel < 0 || zoomLevel >= zoomLevels.size()) {
				throw new RuntimeException("Zoom Level '" + zoomLevel + "' is out of bounds.");
			}
			Object zoomFactorObject = zoomLevels.get(zoomLevel); 
			if (zoomFactorObject instanceof Double) {
				zoomFactor = (Double) zoomFactorObject;				
			} else if (zoomFactorObject instanceof Integer) {
				zoomFactor = ((Integer) zoomFactorObject).doubleValue();				
			} else {
				throw new RuntimeException("Zoom Factor in json is of an incompatible type: " + zoomFactorObject.toString());
			}
		} catch (RuntimeException e) {
			// propagate if not json or null pointer exception
			if (!(e instanceof JSONException) && !(e instanceof NullPointerException)) {
				throw e;
			}
		}

		// augment the path by the zoom level
		tileDir = new File(tileDir, String.valueOf(zoomLevel));

		boolean previewAsBoolean = false; 
		try {
			previewAsBoolean = Boolean.parseBoolean(preview);
		} catch (Exception e) {
			// fall back onto default
		}
		
		int tileSizeAsInt = 256;
		try {
			tileSizeAsInt = Integer.parseInt(tileSize);
		} catch (Exception e) {
			// fall back onto default
		}
		
		// now call the native tilling method and return
		return new RestfulResource(
				new Response(
						jniTissueStack.tileMincVolume(
								dataSet.getFilename(),
								tileDir.getAbsolutePath(),
								dimensionsArray,
								tileSizeAsInt,
								zoomFactor,
								imageType.trim(),
								previewAsBoolean)));
	}

	@Path("/meta-info")
	@Description("Shows MINC Resources Meta Info.")
	public RestfulResource getMINCResourcesMetaInfo() {
		return new RestfulResource(new Response(this.getMetaInfo()));
	}

}
