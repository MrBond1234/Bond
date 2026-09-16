#include "bond/application.hpp"
#include <iostream>
int main() { bond::Application app; if(!app.initialize(BOND_DATA_DIR "/lessons/campaign_v1.psv")) { std::cerr << "Cannot load campaign data\n"; return 1; } std::cout << "Bond Automation Training v0.1 | " << app.campaign().lessons().size() << " lessons loaded\n"; return 0; }
