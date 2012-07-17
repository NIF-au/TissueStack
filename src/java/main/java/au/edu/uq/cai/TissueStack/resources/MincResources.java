package au.edu.uq.cai.TissueStack.resources;

import java.io.File;

import javax.ws.rs.Path;
import javax.ws.rs.QueryParam;

import au.edu.uq.cai.TissueStack.dataobjects.DataSet;
import au.edu.uq.cai.TissueStack.dataobjects.MincInfo;
import au.edu.uq.cai.TissueStack.dataobjects.NoResults;
import au.edu.uq.cai.TissueStack.dataobjects.Response;
import au.edu.uq.cai.TissueStack.dataprovider.DataSetDataProvider;
import au.edu.uq.cai.TissueStack.jni.TissueStack;
import au.edu.uq.cai.TissueStack.rest.AbstractRestfulMetaInformation;
import au.edu.uq.cai.TissueStack.rest.Description;

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

	@Path("/tile")
	@Description("Tiles a given data set")
	public RestfulResource tileDataSet(
			@Description("Parameter 'dataset_id': data set id of the data set to be tiled (Either 'file' or 'dataset_id' have to be given!!!)")
			@QueryParam("dataset_id")
			String dataSetId,
			@Description("Parameter 'file': file name of the data to be tiled (Either 'file' or 'dataset_id' have to be given!!!)")
			@QueryParam("file")
			String mincFile,
			@Description("Optional: dimensions to be tiled (comma separated) default: 0")
			@QueryParam("start")
			String dimensions,
			@Description("Optional: start slices per dimension (comma separated). default: 0")
			@QueryParam("start")
			String starts,
			@Description("Optional: end slices per dimension (comma separated). default: maximum slice number as per dimension")
			@QueryParam("end")
			String ends,
			@Description("Optional: tile as low res preview ('true' or 'false'), default: false")
			@QueryParam("preview")
			String preview
			){
		boolean missingSource = true;
		// look for file
		if (mincFile != null && new File(mincFile).exists()) {
			missingSource = false;
		}
		// look for data set id
		DataSet dataSet = null;
		try {
			dataSet = DataSetDataProvider.queryDataSetById(Long.parseLong(dataSetId));
			if (dataSet != null) missingSource = false;
		} catch(Exception idNotThereOrNotNUmeric) {
			// we ignore that
		}
		// check whether we have either a file or valid id as input
		if (missingSource) {
			throw new IllegalArgumentException(
					"Not a valid source. You have to hand in either an existing data set id or a valid minc file location!");
		}
		
		final TissueStack jniTissueStack = new TissueStack();
		MincInfo relatedMincInfo = null;
		// if we didn't find a data set in the db, query the minc file and populate a data set
		if (dataSet == null) {
			relatedMincInfo = jniTissueStack.getMincInfo(mincFile);
			dataSet = DataSet.fromMincInfo(relatedMincInfo);
		} else {
			relatedMincInfo = jniTissueStack.getMincInfo(dataSet.getFilename());
		}
		// throw an error if we could not create a data set out of the given minc file
		if (dataSet == null) {
			throw new RuntimeException("Could not create data set from given minc file: " + mincFile + ". Check if valid...");
		}
		
		// check rest of params now

		// now call the native tilling method
		jniTissueStack.tileMincVolume(dataSet.getFilename(), null ,0, 0, false);
		
		return new RestfulResource(new Response(dataSet));
	}

	@Path("/meta-info")
	@Description("Shows MINC Resources Meta Info.")
	public RestfulResource getMINCResourcesMetaInfo() {
		return new RestfulResource(new Response(this.getMetaInfo()));
	}

}
