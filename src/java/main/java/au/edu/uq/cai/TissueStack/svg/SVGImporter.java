package au.edu.uq.cai.TissueStack.svg;

import java.io.File;
import java.io.FilenameFilter;
import java.io.IOException;

import org.apache.batik.dom.svg.SAXSVGDocumentFactory;
import org.apache.batik.util.XMLResourceDescriptor;
import org.w3c.dom.NodeList;
import org.w3c.dom.svg.SVGDocument;

import au.edu.uq.cai.TissueStack.dataobjects.DataSet;
import au.edu.uq.cai.TissueStack.dataprovider.DataSetDataProvider;



public class SVGImporter {
	public static void importSvgsFromDirectoryAndLinkThemToADataSet(String dir, long dataSetId) {
		// prelimiary existence checks
		if (dir == null || dir.trim().isEmpty() || dataSetId <=0)
			throw new IllegalArgumentException("A directory location and data set id are mandatory!");
		
		final File directory = new File(dir);
		if (!directory.exists() || !directory.isDirectory() || !directory.canRead())
			throw new RuntimeException("'" + directory.getAbsolutePath() + "' does not exist/is not a readable directory!");
		
		final DataSet dataSet = DataSetDataProvider.queryDataSetById(dataSetId);
		if (dataSet == null)
			throw new RuntimeException("Data Set with id '" + dataSetId + "' could not be found in the database!");
		
		// read content of directory, only svgs are being processed
		File[] listOfSvgFiles = directory.listFiles(new FilenameFilter() {
			public boolean accept(File path, String fileOrDir) {
				if (!fileOrDir.toLowerCase().endsWith(".svg")) 
					return true;
				
				return false;
			}
		});		

		for (File f : listOfSvgFiles) {
			final String absFile = f.getAbsolutePath();
			try {
				SVGImporter.importSvgAndLinkItToDataSet(absFile, dataSet);
			} catch (Exception any) {
				System.out.println("Failed to import file '" + absFile + "'!");
				any.printStackTrace();
			}
		}
	}

		public static void importSvgAndLinkItToDataSet(String file, DataSet dataSet) {
			// prelimiary existence checks
			if (file == null || file.trim().isEmpty() || dataSet == null)
				throw new IllegalArgumentException("An svg file and data set are mandatory!");
			
			final File svgFile = new File(file);
			if (!svgFile.exists() || !svgFile.isFile() || !svgFile.canRead())
				throw new RuntimeException("File '" + svgFile.getAbsolutePath() + "' does not exist/is not a readable file!");
			
			// read and parse the svg content
			SVGDocument svgDocument = null;
			try {
				svgDocument = SVGImporter.openSVGFile(svgFile.getAbsolutePath());
				//String svgContent = SVGImporter.processSVGDocument(svgDocument);
				SVGImporter.processSVGDocument(svgDocument);
				
				// TODO: store SVG content in DB associated with a data set plane
				// double check slice information
			} catch (Exception any) {
				throw new RuntimeException("Failed to open/parse/process SVG file!", any);
			}
		}

		private static SVGDocument openSVGFile(String fileName) throws IOException {
		    final SAXSVGDocumentFactory factory =
		    		new SAXSVGDocumentFactory(XMLResourceDescriptor.getXMLParserClassName());
		    return factory.createSVGDocument("file://" + fileName);
			
		}

		private static String processSVGDocument(SVGDocument svgDocument) {
		    final NodeList list = svgDocument.getElementsByTagName("path");
			
		    // TODO: make this into a good format 
			StringBuffer js = new StringBuffer(
					"var canvas = document.getElementById(\"dataset_1_canvas_y_plane\");\n"
					+ "var context = canvas.getContext(\"2d\");\n"
			);
			
		    js.append("context.lineWidth = 1\n");
		    js.append("context.strokeStyle = \"red\";\n");

			// traverse all path elements
			for (int i=0;i<list.getLength(); i++) {
				// this is to break the outer loop from within a nested loop
				boolean continueMe = false;

				// read the contents of the style attribute
				final String style = list.item(i).getAttributes().getNamedItem("style").getTextContent();
				final String stylePairs[] = (style != null) ? style.split(";") : new String[0];
				for (String pair: stylePairs) // loop through all style key/value pairs
					if (pair.startsWith("fill")) { // we are only interested in shapes that are filled
						final String color[] = pair.split(":");
						if (color.length >= 2 && !color[1].trim().equalsIgnoreCase("none")) {
							js.append("context.fillStyle=\"");
							js.append(color[1].trim());
							js.append("\";\n");
							continueMe = false;
							break;
						}
						continueMe = true;
					}
				
				// this is for paths with fill 'none'
				if (continueMe)
					continue;
				
				// extract the text content of the path element
				String pathD = list.item(i).getAttributes().getNamedItem("d").getTextContent();

				// split coordinates 
				String tokens[] = pathD.split(" ");
				double presentX = 0.0; 
				double presentY = 0.0;

				// TODO: refine that and make it stable
				// we expect paths of a certain kind: bezier curves with M !
				// first one is move to 
				if (!tokens[0].equals("M")) {
					System.out.println("move to ommitted");
				}
				String xyCoords[] = tokens[1].split(",");
				
				presentX = Double.parseDouble(xyCoords[0]);
				presentY = Double.parseDouble(xyCoords[1]);
				
				js.append("context.beginPath();\n");
				js.append("context.moveTo(");
				js.append(presentX + "," + presentY);
				js.append(");\n");

				// TODO: code for relative coords
				if (!tokens[2].equals("C")) {
					System.out.println("c ommitted");
				}
				js.append("context.bezierCurveTo(");
				
				int pointsPerCurve = 0;
				
				for (int x = 3; x<tokens.length; x++) {
					if (pointsPerCurve != 0 && x != tokens.length-1 && (pointsPerCurve % 3) == 0) {
						js.deleteCharAt(js.length()-1);
						js.append(");\n");
						js.append("context.bezierCurveTo(");
					}
					
					String token = tokens[x];
					
					if (token.equals("z")) {
						continue;
					}
					
					xyCoords = token.split(",");
					if (xyCoords.length != 2) {
						System.out.println("xy pair is not existant");
					}
					double xCoord = Double.parseDouble(xyCoords[0]);
					double yCoord = Double.parseDouble(xyCoords[1]);
					
					pointsPerCurve++;

					js.append(xCoord + "," + yCoord);
					js.append(",");
				}

				// get rid of trailing ','
				if (js.charAt(js.length()-1) == ',') {
					js.deleteCharAt(js.length()-1);
					js.append(");\n");
				}

				js.append("context.fill();\n");
			}
		
			return js.toString();
		}

		public static void main(String args[]) {
			try {
				// open svg
				final SVGDocument existingDoc = 
						SVGImporter.openSVGFile("/home/harald/Downloads/2012-05-17_tracings_comb/coronal-0355.svg");
						//SVGImporter.openSVGFile(args[0]);
				System.out.println(SVGImporter.processSVGDocument(existingDoc));
				
			    /*
 				NodeList list = existingDoc.getElementsByTagName("path");

				// create new svg document
				DOMImplementation impl = SVGDOMImplementation.getDOMImplementation();
				String svgNS = SVGDOMImplementation.SVG_NAMESPACE_URI;
				Document newDoc = impl.createDocument(svgNS, "svg", null);

				// Get the root element (the 'svg' element).
				Element svgRoot = newDoc.getDocumentElement();
				Element image = newDoc.createElement("image");
				image.setAttributeNS(null, "x", "0");
				image.setAttributeNS(null, "y", "0");
				image.setAttributeNS(null, "width", "680");
				image.setAttributeNS(null, "height", "500");
				
				svgRoot.appendChild(image);
				
				StringBuffer js = new StringBuffer(
						"var canvas = document.getElementById(\"dataset_1_canvas_y_plane\");\n"
						+ "var context = canvas.getContext(\"2d\");\n"		
				);

			    js.append("context.lineWidth = 1\n");
			    js.append("context.strokeStyle = \"red\";\n");

				int moveToPoints = 0;

				// append all paths with absolute coords
				for (int i=0;i<list.getLength(); i++) {

					boolean continueMe = false;
					
					// create new path element 
					Element path = newDoc.createElement("path");
					// copy style and id
					final String style = list.item(i).getAttributes().getNamedItem("style").getTextContent();
					final String stylePairs[] = (style != null) ? style.split(";") : new String[0];
					for (String pair: stylePairs)
						if (pair.startsWith("fill")) {
							final String color[] = pair.split(":");
							if (color.length >= 2 && !color[1].trim().equalsIgnoreCase("none")) {
								js.append("context.fillStyle=\"");
								js.append(color[1].trim());
								js.append("\";\n");
								continueMe = false;
								break;
							}
							continueMe = true;
						}
					
					if (continueMe)
						continue;
					
					path.setAttributeNS(null, "style", style);
					path.setAttributeNS(null, "id", list.item(i).getAttributes().getNamedItem("id").getTextContent());

					// alter coords then add
					String pathD = list.item(i).getAttributes().getNamedItem("d").getTextContent();

					StringBuffer absolutePathD = new StringBuffer(pathD.length());
					String tokens[] = pathD.split(" ");
					double presentX = 0.0; 
					double presentY = 0.0;

					moveToPoints++;
					
					// first one is move to 
					if (!tokens[0].equals("M")) {
						System.out.println("move to ommitted");
					}
					absolutePathD.append("M ");
					absolutePathD.append(tokens[1]);
					
					String xyCoords[] = tokens[1].split(",");
					
					presentX = Double.parseDouble(xyCoords[0]);
					presentY = Double.parseDouble(xyCoords[1]);
					
					js.append("context.beginPath();\n");
					js.append("context.moveTo(");
					js.append(presentX + "," + presentY);
					js.append(");\n");
					
					if (!tokens[2].equals("C")) {
						System.out.println("c ommitted");
					}
					absolutePathD.append(" C");
					js.append("context.bezierCurveTo(");
					
					int pointsPerCurve = 0;
					
					for (int x = 3; x<tokens.length; x++) {
						if (pointsPerCurve != 0 && x != tokens.length-1 && (pointsPerCurve % 3) == 0) {
							js.deleteCharAt(js.length()-1);
							js.append(");\n");
							js.append("context.bezierCurveTo(");
						}
						
						String token = tokens[x];
						
						if (token.equals("z")) {
							absolutePathD.append(" Z");
							continue;
						}
						
						xyCoords = token.split(",");
						if (xyCoords.length != 2) {
							System.out.println("xy pair is not existant");
						}
						double xCoord = Double.parseDouble(xyCoords[0]);
						double yCoord = Double.parseDouble(xyCoords[1]);
						
						pointsPerCurve++;

						js.append(xCoord + "," + yCoord);
						js.append(",");
						
						absolutePathD.append(" " + xCoord + "," + yCoord);
					}
					System.out.println("Points Per Curve: " + pointsPerCurve);
					
					if (js.charAt(js.length()-1) == ',') {
						js.deleteCharAt(js.length()-1);
						js.append(");\n");
					}

					js.append("context.fill();\n");

					path.setAttributeNS(null, "d", absolutePathD.toString());
					svgRoot.appendChild(path);
				}
				
				Transformer transformer = TransformerFactory.newInstance().newTransformer();
				Result output = new StreamResult(new FileOutputStream(new File("/home/harald/Downloads/2012-05-17_tracings_comb/coronal-0347-changed.svg")));
				Source input = new DOMSource(newDoc);

				transformer.transform(input, output);

				System.out.println("Move To Points: " + moveToPoints);
		        
				System.out.println(js.toString());
				*/
			} catch (IOException ex) {
			    ex.printStackTrace();
			}
		}
}
