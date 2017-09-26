#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Algorithms/GraphTraversal/StronglyConnectedComponents.h"
#include "DataStructures/Graph/Attributes/CapacityAttribute.h"
#include "DataStructures/Graph/Attributes/CoordinateAttribute.h"
#include "DataStructures/Graph/Attributes/FreeFlowSpeedAttribute.h"
#include "DataStructures/Graph/Attributes/LatLngAttribute.h"
#include "DataStructures/Graph/Attributes/LengthAttribute.h"
#include "DataStructures/Graph/Attributes/NumLanesAttribute.h"
#include "DataStructures/Graph/Attributes/TravelTimeAttribute.h"
#include "DataStructures/Graph/Attributes/VertexIdAttribute.h"
#include "DataStructures/Graph/Attributes/XatfRoadCategoryAttribute.h"
#include "DataStructures/Graph/Export/DefaultExporter.h"
#include "DataStructures/Graph/Graph.h"
#include "DataStructures/Graph/Import/VisumImporter.h"
#include "DataStructures/Graph/Import/XatfImporter.h"
#include "Tools/CommandLine/CommandLineParser.h"
#include "Tools/ContainerHelpers.h"

// A graph data structure encompassing all vertex and edge attributes available for output.
using VertexAttributes = VertexAttrs<CoordinateAttribute, LatLngAttribute, VertexIdAttribute>;
using EdgeAttributes = EdgeAttrs<
    CapacityAttribute, FreeFlowSpeedAttribute, LengthAttribute,
    NumLanesAttribute, TravelTimeAttribute, XatfRoadCategoryAttribute>;
using GraphT = StaticGraph<VertexAttributes, EdgeAttributes>;

void printUsage() {
  std::cout <<
      "Usage: ConvertGraph -s <fmt> -d <fmt> [-c] [-scc] -a <attrs> -i <file> -o <file>\n"
      "This program converts a graph from a source file format to a destination format,\n"
      "possibly extracting the largest strongly connected component of the input graph.\n"
      "  -s <fmt>          source file format\n"
      "                      possible values: binary default dimacs visum xatf\n"
      "  -d <fmt>          destination file format\n"
      "                      possible values: binary default dimacs\n"
      "  -c                compress the output file(s), if available\n"
      "  -scc              extract the largest strongly connected component\n"
      "  -ts <sys>         the system whose network is to be imported (Visum only)\n"
      "  -cs <epsg-code>   input coordinate system (Visum only)\n"
      "  -ap <hours>       analysis period, capacity is in vehicles/AP (Visum only)\n"
      "  -a <attrs>        blank-separated list of vertex/edge attributes to be output\n"
      "                      possible values:\n"
      "                        capacity coordinate free_flow_speed lat_lng length\n"
      "                        num_lanes travel_time vertex_id xatf_road_category\n"
      "  -i <file>         input file(s) without file extension\n"
      "  -o <file>         output file(s) without file extension\n"
      "  -help             display this help and exit\n";
}

// Prints the specified error message to standard error.
void printErrorMessage(const std::string& invokedName, const std::string& msg) {
  std::cerr << invokedName << ": " << msg << std::endl;
  std::cerr << "Try '" << invokedName <<" -help' for more information." << std::endl;
}

// Imports a graph according to the input file format specified on the command line and returns it.
GraphT importGraph(const CommandLineParser& clp) {
  const std::string fmt = clp.getValue<std::string>("s");
  const std::string infile = clp.getValue<std::string>("i");

  // Pick the appropriate import procedure.
  if (fmt == "binary") {
    std::ifstream in(infile + ".gr.bin", std::ios::binary);
    if (!in.good())
      throw std::invalid_argument("file not found -- '" + infile + ".gr.bin'");
    return GraphT(in);
  } else if (fmt == "visum") {
    const std::string sys = clp.getValue<std::string>("ts", "P");
    const int crs = clp.getValue<int>("cs", 31467);
    const int ap = clp.getValue<int>("ap", 24);
    if (ap <= 0) {
      const auto what = "analysis period not strictly positive -- '" + std::to_string(ap) + "'";
      throw std::invalid_argument(what);
    }
    return GraphT(infile, VisumImporter(infile, sys, crs, ap));
  } else if (fmt == "xatf") {
    return GraphT(infile, XatfImporter());
  } else {
    throw std::invalid_argument("unrecognized input file format -- '" + fmt + "'");
  }
}

// Executes a graph export using the specified exporter.
template <typename ExporterT>
void doExport(const CommandLineParser& clp, const GraphT& graph, ExporterT ex) {
  // Output only those attributes specified on the command line.
  std::vector<std::string> attrsToOutput = clp.getValues<std::string>("a");
  for (const auto& attr : GraphT::getAttributeNames())
    if (!contains(attrsToOutput.begin(), attrsToOutput.end(), attr))
      ex.ignoreAttribute(attr);
  graph.exportTo(clp.getValue<std::string>("o"), ex);
}

// Exports the specified graph according to the output file format specified on the command line.
void exportGraph(const CommandLineParser& clp, const GraphT& graph) {
  const std::string fmt = clp.getValue<std::string>("d");
  const bool compress = clp.isSet("c");

  // Pick the appropriate export procedure.
  if (fmt == "binary") {
    const std::string outfile = clp.getValue<std::string>("o");
    std::ofstream out(outfile + ".gr.bin", std::ios::binary);
    if (!out.good())
      throw std::invalid_argument("file cannot be opened -- '" + outfile + ".gr.bin'");
    // Output only those attributes specified on the command line.
    std::vector<std::string> attrsToIgnore;
    std::vector<std::string> attrsToOutput = clp.getValues<std::string>("a");
    for (const auto& attr : GraphT::getAttributeNames())
      if (!contains(attrsToOutput.begin(), attrsToOutput.end(), attr))
        attrsToIgnore.push_back(attr);
    graph.writeTo(out, attrsToIgnore);
  } else if (fmt == "default") {
    doExport(clp, graph, DefaultExporter(compress));
  } else {
    throw std::invalid_argument("unrecognized output file format -- '" + fmt + "'");
  }
}

int main(int argc, char* argv[]) {
  CommandLineParser clp;
  try {
    clp.parse(argc, argv);
  } catch (std::invalid_argument& e) {
    printErrorMessage(argv[0], e.what());
    return EXIT_FAILURE;
  }

  if (clp.isSet("help")) {
    printUsage();
    return EXIT_SUCCESS;
  }

  try {
    std::cout << "Reading the input file(s)..." << std::flush;
    GraphT graph = importGraph(clp);
    std::cout << " done." << std::endl;

    if (clp.isSet("scc")) {
      std::cout << "Computing strongly connected components..." << std::flush;
      StronglyConnectedComponents scc;
      scc.run(graph);
      std::cout << " done." << std::endl;

      std::cout << "Extracting the largest SCC..." << std::flush;
      graph.extractVertexInducedSubgraph(scc.getLargestSccAsBitmask());
      std::cout << " done." << std::endl;
    }
    if (clp.isSet("o")) {
      std::cout << "Writing the output file(s)..." << std::flush;
      exportGraph(clp, graph);
      std::cout << " done." << std::endl;
    }
  } catch (std::invalid_argument& e) {
    printErrorMessage(argv[0], e.what());
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
