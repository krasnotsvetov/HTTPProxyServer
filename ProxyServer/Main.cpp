#include <iostream>
#include "ProxyServer.h"
#pragma comment(lib, "Ws2_32.lib")

int main() {

	bool isServerRun = true;
	ProxyServer pServer(3000, 7777);

	pServer.Run();


	while (isServerRun) {

		int t;
		std::cin >> t;
		if (t == -1) {
			isServerRun = false;

		}
		if (t == -2) {
			pServer.Stop();
			isServerRun = false;
		}
	}
	return 0;
}