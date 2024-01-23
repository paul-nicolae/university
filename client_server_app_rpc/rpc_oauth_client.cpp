#include "rpc_oauth.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "token.h"
#include <map>

using namespace std;

// define a global map between user_id and access_token
map<string, string> user_id_access_token;

void
rpc_oauth_prog_1(char *host, char *input_file)
{
	CLIENT *clnt;
	res_auth  *result_1;
	req_auth  request_authorization_1_arg;
	res_acc_token  *result_2;
	req_acc_token  request_access_token_1_arg;
	res_val_del_act  *result_3;
	req_val_del_act  validate_delegated_action_1_arg;
	res_approve_req_token  *result_4;
	req_approve_req_token  approve_request_token_1_arg;

#ifndef	DEBUG
	clnt = clnt_create (host, RPC_OAUTH_PROG, RPC_OAUTH_VERS, "udp");
	if (clnt == NULL) {
		clnt_pcreateerror (host);
		exit (1);
	}
#endif	/* DEBUG */

    ifstream inputFile(input_file);

    if (!inputFile.is_open()) {
        cerr << "Error opening the file." << endl;
        return ;
    }

    // read the file line by line
    string line;
    while (getline(inputFile, line)) {
        // use a stringstream to split the line into three words
        istringstream ss(line);
        string user_id, request, resource;
		
        // read the three words separated by commas
        getline(ss, user_id, ',');
        getline(ss, request, ',');
        getline(ss, resource, ',');

		if (strcmp(request.c_str(), "REQUEST") == 0) {
			// req auth
			request_authorization_1_arg.user_id  = new char[TOKEN_LEN + 1];
			strcpy(request_authorization_1_arg.user_id, user_id.c_str());

			result_1 = request_authorization_1(&request_authorization_1_arg, clnt);
			if (result_1 == (res_auth *) NULL) {
				clnt_perror (clnt, "call failed");
			}

			if (result_1->err == USER_NOT_FOUND) {
				cout << "USER_NOT_FOUND" << endl;
				continue;
			}
			
			// approve req token
			approve_request_token_1_arg.auth_token  = new char[TOKEN_LEN + 1];
			strcpy(approve_request_token_1_arg.auth_token, result_1->auth_token);

			result_4 = approve_request_token_1(&approve_request_token_1_arg, clnt);
			if (result_4 == (res_approve_req_token *) NULL) {
				clnt_perror (clnt, "call failed");
			}

			// req acc token
			request_access_token_1_arg.user_id  = new char[TOKEN_LEN + 1];
			strcpy(request_access_token_1_arg.user_id, user_id.c_str());

			request_access_token_1_arg.auth_token  = new char[TOKEN_LEN + 1];
			strcpy(request_access_token_1_arg.auth_token, result_4->auth_token);

			result_2 = request_access_token_1(&request_access_token_1_arg, clnt);
			if (result_2 == (res_acc_token *) NULL) {
				clnt_perror (clnt, "call failed");
			}

			if (result_2->err == REQUEST_DENIED) {
				cout << "REQUEST_DENIED" << endl;
			} else {
				cout << result_1->auth_token << " -> " << result_2->acc_token << endl;
			}

			user_id_access_token[user_id] = string(result_2->acc_token);

		} else {
			// validate delegated action
			validate_delegated_action_1_arg.acc_token  = new char[TOKEN_LEN + 1];
			strcpy(validate_delegated_action_1_arg.acc_token, user_id_access_token[user_id].c_str());

			validate_delegated_action_1_arg.resource  = new char[TOKEN_LEN + 1];
			strcpy(validate_delegated_action_1_arg.resource, resource.c_str());

			if (strcmp(request.c_str(), "READ") == 0) {
				validate_delegated_action_1_arg.op = READ;
			} else if (strcmp(request.c_str(), "INSERT") == 0) {
				validate_delegated_action_1_arg.op = INSERT;
			} else if (strcmp(request.c_str(), "MODIFY") == 0) {
				validate_delegated_action_1_arg.op = MODIFY;
			} else if (strcmp(request.c_str(), "DELETE") == 0) {
				validate_delegated_action_1_arg.op = DELETE;
			} else if (strcmp(request.c_str(), "EXECUTE") == 0) {
				validate_delegated_action_1_arg.op = EXECUTE;
			}
			
			result_3 = validate_delegated_action_1(&validate_delegated_action_1_arg, clnt);
			if (result_3 == (res_val_del_act *) NULL) {
				clnt_perror (clnt, "call failed");
			}
			
			if (result_3->err == PERMISSION_DENIED) {
				cout << "PERMISSION_DENIED" << endl;
			} else if (result_3->err == TOKEN_EXPIRED) {
				cout << "TOKEN_EXPIRED" << endl;
			} else if (result_3->err == RESOURCE_NOT_FOUND) {
				cout << "RESOURCE_NOT_FOUND" << endl;
			} else if (result_3->err == OPERATION_NOT_PERMITTED) {
				cout << "OPERATION_NOT_PERMITTED" << endl;
			} else if (result_3->err == PERMISSION_GRANTED) {
				cout << "PERMISSION_GRANTED" << endl;
			}
		}
    }

    inputFile.close();

#ifndef	DEBUG
	clnt_destroy (clnt);
#endif	 /* DEBUG */
}

int
main (int argc, char *argv[])
{
	char *host;

	if (argc < 2) {
		printf ("usage: %s server_host\n", argv[0]);
		exit (1);
	}

	host = argv[1];
	rpc_oauth_prog_1 (host, argv[2]);


exit (0);
}
