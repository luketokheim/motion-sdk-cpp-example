#include <Client.hpp>
#include <Format.hpp>

#include <fstream>
#include <iostream>
#include <string>

const std::string Separator = ",";
const std::string Newline = "\n";

using Motion::SDK::Client;
using Motion::SDK::Format;


int example(std::ostream *output, const std::string &host, const unsigned &port)
{
  // Open connection to the configurable data service.
  Client client(host, port);
  if (client.isConnected()) {
    std::cout
      << "Connected to " << host << ":" << port
      << Newline;
  } else {
    std::cout
      << "Failed to connect to Motion Service on " << host << ":" << port
      << Newline;
    return 1;
  }

  // Request the channels that we want from every connected device. The full
  // list is available here:
  //   http://www.motionnode.com/tools/configurable.xml
  //
  // We are selecting the global quaterion (Gq) and calibrated accelerometer
  // measurements (a) channels here. 7 numbers per device per frame.
  const std::string xml_definition =
    "<?xml version=\"1.0\"?>"
    "<configurable>"
    "<Gq/>"
    "<a/>"
    "</configurable>";

  if (client.writeData(Client::data_type(xml_definition.begin(), xml_definition.end()))) {
    std::cout
      << "Sent active channel definition to Configurable service"
      << Newline;
  }

  if (client.waitForData()) {
    for (;;) {
      // Read one frame of data from all connected devices.
      Client::data_type data;
      if (!client.readData(data)) {
        std::cout
          << "Data stream interrupted or timed out"
          << Newline;
        break;
      }

      bool have_output_line = false;
      
      // Iterate through the entries, one per device.
      auto list = Format::Configurable(data.begin(), data.end());
      for (auto &item : list) {
        // Iterate through the channels per device.
        for (std::size_t i=0; i<item.second.size(); ++i) {
          if (have_output_line) {
            *output << Separator;
          } else {
            have_output_line = true;
          }

          *output << item.second[i];
        }
      }

      if (have_output_line) {
        *output << Newline;
      } else {
        std::cout
          << "Unknown data format in stream"
          << Newline;
        break;
      }
    }
  } else {
    std::cout
      << "No active data stream available, giving up"
      << Newline;
  }

  return 0;
}

int main(int argc, char **argv)
{
  std::ofstream fout("out.csv", std::ios_base::binary | std::ios_base::out);

  return example(&fout, "127.0.0.1", 32076);
}
