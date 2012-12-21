package au.edu.uq.cai.TissueStack.svg;

import java.io.File;
import java.io.FileReader;
import java.io.FilenameFilter;
import java.io.IOException;

import org.apache.batik.dom.svg.SAXSVGDocumentFactory;
import org.apache.batik.util.XMLResourceDescriptor;
import org.w3c.dom.NodeList;
import org.w3c.dom.svg.SVGDocument;

import au.edu.uq.cai.TissueStack.JPAUtils;
import au.edu.uq.cai.TissueStack.dataobjects.AbstractDataSetOverlay;
import au.edu.uq.cai.TissueStack.dataobjects.CanvasOverlay;
import au.edu.uq.cai.TissueStack.dataobjects.DataSet;
import au.edu.uq.cai.TissueStack.dataobjects.DataSetPlanes;
import au.edu.uq.cai.TissueStack.dataobjects.SVGOverlay;
import au.edu.uq.cai.TissueStack.dataprovider.DataSetDataProvider;
import au.edu.uq.cai.TissueStack.dataprovider.DataSetOverlaysProvider;

public class SVGImporter {
		private static SVGDocument openSVGFile(String fileName) throws IOException {
		    final SAXSVGDocumentFactory factory =
		    		new SAXSVGDocumentFactory(XMLResourceDescriptor.getXMLParserClassName());
		    return factory.createSVGDocument("file://" + fileName);
			
		}

		/*
		 * SVG stuff is transalated into 1 long string of key/value pairs separated by: |
		 *	Conversion Legend:
		 *	FS .... fillStyle
		 *  F  .... fill
		 *	P  .... beginPath
		 *	M  .... moveTo
		 *  L  .... lineTo
		 *  C  .... cubic bezier
		 *  
		 *  .... more to come ...
		 *  
		 *  With param 'convertIntoJs' TRUE the output will be copy & pasteable js,
		 *  With param 'convertIntoJs' TRUE the output will correspond to the explanation and legend above
		 */
		private static String processSVGDocument(SVGDocument svgDocument, boolean convertIntoJs) {
		    final NodeList list = svgDocument.getElementsByTagName("path");
			
			StringBuffer js = new StringBuffer(500);

			/*
			if (convertIntoJs) {
				js.append("var canvas = document.getElementById(\"dataset_1_canvas_y_plane\");\n");
				js.append("var context = canvas.getContext(\"2d\");\n");
			}*/

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
							if (convertIntoJs)
								js.append("context.fillStyle=\"");
							else 
								js.append("FS=");
							
							js.append(color[1].trim());
							
							if (convertIntoJs)
								js.append("\";\n");
							else 
								js.append("|");
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

				// BIG TODO: rewrite the following abomination of a code segment with nice separations bewtween the PATH types: C,M,L,Z,V,H, S, Q, T
				
				// split coordinates 
				String tokens[] = pathD.split(" ");
				double presentX = 0.0; 
				double presentY = 0.0;

				// we expect paths of a certain kind: bezier curves with M !
				// first one is move to 
				if (!tokens[0].equalsIgnoreCase("M")) {
					System.out.println("move to ommitted");
				}
				String xyCoords[] = tokens[1].split(",");
				
				presentX = Double.parseDouble(xyCoords[0]);
				presentY = Double.parseDouble(xyCoords[1]);
				
				if (convertIntoJs) {
					js.append("context.beginPath();\n");
					js.append("context.moveTo(");
					js.append(presentX + "," + presentY);
					js.append(");\n");
				} else {
					js.append("P=|");
					js.append("M=");
					js.append(presentX + "," + presentY);
					js.append("|");
				}

				if (!tokens[2].equalsIgnoreCase("C")) {
					System.out.println("c ommitted");
				}
				
				if (convertIntoJs) 
					js.append("context.bezierCurveTo(");
				else 
					js.append("C=");
				
				int pointsPerCurve = 0;
				
				for (int x = 3; x<tokens.length; x++) {
					String token = tokens[x];
					
					if (token.equalsIgnoreCase("Z")) {
						continue;
					} else if (token.equalsIgnoreCase("M")) {
						js.deleteCharAt(js.length()-1);
						if (convertIntoJs) {
							js.append(");\n");
							js.append("context.moveTo(");
						} else
							js.append("|M=");

						token = tokens[++x];
					} else if (token.equalsIgnoreCase("L")) {
						js.deleteCharAt(js.length()-1);
						if (convertIntoJs) {
							js.append(");\n");
							js.append("context.lineTo(");
						} else
							js.append("|L=");

						token = tokens[++x];
					} else	if (token.equalsIgnoreCase("C") ||
							(pointsPerCurve != 0 && x != tokens.length-1 && (pointsPerCurve % 3) == 0)) {
						js.deleteCharAt(js.length()-1);
						if (convertIntoJs) {
							js.append(");\n");
							js.append("context.bezierCurveTo(");
						} else 
							js.append("|C=");
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
					if (convertIntoJs)
						js.append(");\n");
					else
						js.append("|");
				}

				if (convertIntoJs)
					js.append("context.fill();\n");
				else
					js.append("F=|");
			}
		
			return js.toString();
		}

		/*
		 * This main method looks at the contents of a given directory and either imports them whole into the database
		 * or converts it to an internal string that can then be used to draw via canvas methods
		 * 
		 * The first parameter is mandatory and determines whether the SVG should be imported ('IMPORT') or converted ('CONVERT') 
		 * The second input parameter should be directory pointing towards svg files (optional). The current directory is the default. 
		 * 
		 * The necessary naming convention that needs to be adhered to is: "dsid-planeid-slicenr.svg", e.g. 1-y-0129.svg
		 * Otherwise files will be ignored!
		 */
		public static void main(String args[]) {
			if (args.length == 0 || !(args[0].equalsIgnoreCase("IMPORT") || args[0].equalsIgnoreCase("CONVERT"))) {
				System.out.println("Please determine the mode: 'IMPORT' for import, 'CONVERT' for conversion");
				System.exit(-1);
			}
			final String mode = args[0];
			
			final File dir = new File(args.length > 1 ? args[1] : "./");
			String svgFiles[] = null;
			if (!dir.exists() || !dir.isDirectory() || !dir.canRead())
				throw new IllegalArgumentException("Location '" + dir.getAbsolutePath() + "' does not exist/is not a directory/is not readable!");
			
			svgFiles = dir.list(new FilenameFilter() {
				public boolean accept(File dir, String name) {
					File f = new File(dir, name);
					if (!name.endsWith(".svg") || !f.isFile() || !f.canRead())
						return false;
					
					return true;
				}
			});
			
			if (svgFiles == null || svgFiles.length == 0) {
				System.out.println("No suitable candidates for an svg import found");
			}

			// initialize JPAUtils for standalone jdbc connections
			JPAUtils.standAloneInstance();
			
			for (String svgFile : svgFiles) {
				// extract dataset id and, plane id and slice id from file name
				long id = -1;
				String plane =null;
				int slice = -1;
				DataSetPlanes dataSetPlaneFound = null;
				
				try {
					String tokens[] = svgFile.split("-");
					if (tokens.length != 3)
						throw new IllegalAccessException("Name does not contain 3 hyphen separated values!");
					
					id = Long.parseLong(tokens[0]);
					plane = tokens[1];
					slice = Integer.parseInt(tokens[2].substring(0, tokens[2].indexOf(".svg")));
					
					// now check values against dataset in database
					DataSet dataSetFound = DataSetDataProvider.queryDataSetById(id);
					if (dataSetFound == null)
						throw new IllegalArgumentException("No Data Set found for id: " + id);
					
					for (DataSetPlanes p : dataSetFound.getPlanes()) 
						if (p.getName().equalsIgnoreCase(plane)) {
							dataSetPlaneFound = p;
							break;
						}
					
					if (dataSetPlaneFound == null)
						throw new IllegalArgumentException("No Data Set Plane found for id: " + plane);
						
					if (!(slice >= 0 && slice <= dataSetPlaneFound.getMaxSlices()))
						throw new IllegalArgumentException("Slice " + slice + " does not lie within the bounds of the plane!");

					AbstractDataSetOverlay overlay = null;
					
					if (mode.equalsIgnoreCase("CONVERT")) {
						// open svg
						final SVGDocument existingDoc = 
								SVGImporter.openSVGFile(new File(dir, svgFile).getAbsolutePath());
						String content = SVGImporter.processSVGDocument(existingDoc, true);

						if (content != null && !content.isEmpty()) {
							CanvasOverlay canvasOverlay = new CanvasOverlay();
							canvasOverlay.setDataSetId(dataSetPlaneFound.getDatasetId());
							canvasOverlay.setDataSetPlaneId(dataSetPlaneFound.getId());
							canvasOverlay.setSlice(slice);
							canvasOverlay.setName("Canvas Overlay");
							canvasOverlay.setContent(content);
							overlay = canvasOverlay;
						}
					} else if (mode.equalsIgnoreCase("IMPORT")) {
						FileReader reader = null;
						char buffer[] = null;
						
						try {
							// Note: it is assumed the files are fairly small => we read them in one go !
							File fileToBeRead = new File(dir, svgFile);
							reader = new FileReader(fileToBeRead);
							buffer = new char[(int)fileToBeRead.length()]; 
							reader.read(buffer);
						} finally {
							try {
								reader.close();
							} catch (Exception e) {
								// nothing we can do here
							}
						}
						
						if (buffer != null) {
							SVGOverlay svgOverlay = new SVGOverlay();
							svgOverlay.setDataSetId(dataSetPlaneFound.getDatasetId());
							svgOverlay.setDataSetPlaneId(dataSetPlaneFound.getId());
							svgOverlay.setSlice(slice);
							svgOverlay.setName("SVG Overlay");
							svgOverlay.setContent(new String(buffer));
							overlay = svgOverlay;
						}
					}
					
					// persist to data base
					if (overlay != null)
						DataSetOverlaysProvider.insertOverlay(overlay);
				} catch (Exception e) {
					System.out.println("Failed to process file: " + svgFile);
					e.printStackTrace();
					continue;
				}
			}
		}
}
