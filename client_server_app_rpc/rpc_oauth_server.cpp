#include "rpc_oauth.h"
#include <string>
#include <iostream>
#include <fstream>
#include "token.h"
#include <sstream>
#include <map>
#include <vector>
#include <queue>
#include <algorithm>

using namespace std;

class Client {
public:
	string user_id;
	string auth_token;
	string signed_auth_token;
	string access_token;
	int ttl_access_token;
	string refresh_token;
	map<string, vector<int>> resource_operations;

	Client(string user_id, string auth_token) {
		this->user_id = user_id;
		this->auth_token = auth_token;
		this->signed_auth_token = "";
		this->access_token = "";
		this->ttl_access_token = 0;
		this->refresh_token = "";
		this->resource_operations = map<string, vector<int>>();
	}
};

// defina a global vector of users in database
vector<string> users_in_database;

// define a global vector of resources
vector<string> resources;

// define a global queue of maps between resource and a vector of operations
queue<map<string, vector<int>>> resource_operations_queue;

// define a global map between user_id and Client object
map<string, Client*> user_id_clients;

int ttl;

void parse_user_id_file(string user_id_file) {
	ifstream file(user_id_file);

	if (!file.is_open()) {
		cerr << "Error opening user id file: " << user_id_file << endl;
		return;
	}

	int num_lines;
	file >> num_lines;

	// ignore the newline character after reading the number of lines
	file.ignore();

	// read each user id from the file
	for (int i = 0; i < num_lines; ++i) {
		string user_id;
		getline(file, user_id);
		users_in_database.push_back(user_id);
	}

	file.close();
}

void parse_resource_file(string resource_file) {
	ifstream file(resource_file);

	if (!file.is_open()) {
		cerr << "Error opening resources file: " << resource_file << endl;
		return;
	}

	int num_lines;
	file >> num_lines;

	// ignore the newline character after reading the number of lines
	file.ignore();

	// read each resource from the file
	for (int i = 0; i < num_lines; ++i) {
		string resource;
		getline(file, resource);
		resources.push_back(resource);
	}

	file.close();
}

void parse_approvals_file(string approvals_file) {
	ifstream file(approvals_file);

	if (!file.is_open()) {
		cerr << "Error opening resources file: " << approvals_file << endl;
		return;
	}

	// read each line from the file
    string line;
    while (getline(file, line)) {
        // use stringstream to separate the line by commas
        stringstream ss(line);
		string resource;

		map<string, vector<int>> resource_operations;
        while (getline(ss, resource, ',')) {
            if (resource == "*") {
				// the client does not have access to any resource
				resource_operations["*"] = vector<int>();
			} else {
				string permissions;
				getline(ss, permissions, ',');
				for (char perm : permissions) {
					if (perm == 'R') {
						resource_operations[resource].push_back(READ);
					} else if (perm == 'I'){
						resource_operations[resource].push_back(INSERT);
					} else if (perm == 'M') {
						resource_operations[resource].push_back(MODIFY);
					} else if (perm == 'D') {
						resource_operations[resource].push_back(DELETE);
					} else if (perm == 'X') {
						resource_operations[resource].push_back(EXECUTE);
					}
				}
			}
			
        }
		// push the resource_operations map to the queue
		resource_operations_queue.push(resource_operations);
    }

	file.close();
}

string choose_operation(int op) {
	if (op == READ) {
		return "READ";
	} else if (op == INSERT) {
		return "INSERT";
	} else if (op == MODIFY) {
		return "MODIFY";
	} else if (op == DELETE) {
		return "DELETE";
	} else if (op == EXECUTE) {
		return "EXECUTE";
	}
}

static void
rpc_oauth_prog_1(struct svc_req *rqstp, register SVCXPRT *transp)
{
	union {
		req_auth request_authorization_1_arg;
		req_acc_token request_access_token_1_arg;
		req_val_del_act validate_delegated_action_1_arg;
		req_approve_req_token approve_request_token_1_arg;
	} argument;
	char *result;
	xdrproc_t _xdr_argument, _xdr_result;
	char *(*local)(char *, struct svc_req *);

	switch (rqstp->rq_proc) {
	case NULLPROC:
		(void) svc_sendreply (transp, (xdrproc_t) xdr_void, (char *)NULL);
		return;

	case request_authorization:
		_xdr_argument = (xdrproc_t) xdr_req_auth;
		_xdr_result = (xdrproc_t) xdr_res_auth;
		local = (char *(*)(char *, struct svc_req *)) request_authorization_1_svc;
		break;

	case request_access_token:
		_xdr_argument = (xdrproc_t) xdr_req_acc_token;
		_xdr_result = (xdrproc_t) xdr_res_acc_token;
		local = (char *(*)(char *, struct svc_req *)) request_access_token_1_svc;
		break;

	case validate_delegated_action:
		_xdr_argument = (xdrproc_t) xdr_req_val_del_act;
		_xdr_result = (xdrproc_t) xdr_res_val_del_act;
		local = (char *(*)(char *, struct svc_req *)) validate_delegated_action_1_svc;
		break;

	case approve_request_token:
		_xdr_argument = (xdrproc_t) xdr_req_approve_req_token;
		_xdr_result = (xdrproc_t) xdr_res_approve_req_token;
		local = (char *(*)(char *, struct svc_req *)) approve_request_token_1_svc;
		break;

	default:
		svcerr_noproc (transp);
		return;
	}
	memset ((char *)&argument, 0, sizeof (argument));
	if (!svc_getargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
		svcerr_decode (transp);
		return;
	}
	result = (*local)((char *)&argument, rqstp);
	if (result != NULL && !svc_sendreply(transp, (xdrproc_t) _xdr_result, result)) {
		svcerr_systemerr (transp);
	}
	if (!svc_freeargs (transp, (xdrproc_t) _xdr_argument, (caddr_t) &argument)) {
		fprintf (stderr, "%s", "unable to free arguments");
		exit (1);
	}
	return;
}

res_auth *
request_authorization_1_svc(req_auth *argp, struct svc_req *rqstp)
{
	static res_auth result;

	// check if the user id is in the users_in_database vector
	string user_id(argp->user_id);
	if (find(users_in_database.begin(), users_in_database.end(), user_id) == users_in_database.end()) {
		result.err = USER_NOT_FOUND;
		cout << "BEGIN " << argp->user_id << " AUTHZ" << endl;
	} else {
		// generate a new auth token and store it in the user_id_clients map
		char *auth_token = new char[TOKEN_LEN + 1];
		strcpy(auth_token, generate_access_token(argp->user_id));

		// create a new Client object and store it in the user_id_clients map
		Client *client = new Client(string(argp->user_id), string(auth_token));
		user_id_clients[string(argp->user_id)] = client;

		result.auth_token = new char[TOKEN_LEN + 1];
		strcpy(result.auth_token, auth_token);
		result.err = NO_ERROR;
	}

	return &result;
}

res_approve_req_token *
approve_request_token_1_svc(req_approve_req_token *argp, struct svc_req *rqstp)
{
	static res_approve_req_token result;

	string auth_token(argp->auth_token);
	// itereate through every clinet in the user_id_map and stop when the auth token is found
	for (auto it = user_id_clients.begin(); it != user_id_clients.end(); ++it) {
		if (it->second->auth_token == auth_token) {
			// assign to this client the front of the resource_operations_queue
			it->second->resource_operations = resource_operations_queue.front();
			// pop the front of the resource_operations_queue
			resource_operations_queue.pop();

			// verify if the user approved the request
			// if so, return the auth token signed, otherwise return the exact same auth token
			if (it->second->resource_operations.find("*") == it->second->resource_operations.end()) {
				// return a new signed auth token
				result.auth_token = new char[TOKEN_LEN + 1];
				strcpy(result.auth_token, argp->auth_token);
				result.auth_token[0] = 'S';
				result.auth_token[1] = 'S';
				user_id_clients[it->first]->signed_auth_token = string(result.auth_token);
			} else {
				// return the exact same auth token
				result.auth_token = new char[TOKEN_LEN + 1];
				strcpy(result.auth_token, argp->auth_token);
			}
			break;
		}
	}

	return &result;
}

res_acc_token *
request_access_token_1_svc(req_acc_token *argp, struct svc_req *rqstp)
{
	static res_acc_token  result;

	result.acc_token = new char[TOKEN_LEN + 1];
	result.regen_acc_token = new char[TOKEN_LEN + 1];

	// check if the auth token from the client is not signed by analyzing the first two characters	
	if (argp->auth_token[0] != 'S' && argp->auth_token[1] != 'S') {
		result.err = REQUEST_DENIED;
	} else {
		result.acc_token = generate_access_token(user_id_clients[string(argp->user_id)]->auth_token.c_str());
		result.err = NO_ERROR;

		// store the access token in the client object
		user_id_clients[string(argp->user_id)]->access_token = string(result.acc_token);
		user_id_clients[string(argp->user_id)]->ttl_access_token = ttl;
	}

	result.ttl = ttl;

	cout << "BEGIN " << argp->user_id << " AUTHZ" << endl;
	cout << "  RequestToken = " << user_id_clients[string(argp->user_id)]->auth_token << endl;
	if (result.err != REQUEST_DENIED) cout << "  AccessToken = " << result.acc_token << endl;

	return &result;
}

res_val_del_act *
validate_delegated_action_1_svc(req_val_del_act *argp, struct svc_req *rqstp)
{
	static res_val_del_act  result;

	// decrement the ttl of the access token
	string access_token(argp->acc_token);
	for (auto it = user_id_clients.begin(); it != user_id_clients.end(); ++it) {
		if (it->second->access_token == access_token) {
			--it->second->ttl_access_token;
			break;
		}
	}

	// verify if the access token is associated with a client in the user_id_clients map
	if (find_if(user_id_clients.begin(), user_id_clients.end(),
		[&access_token](const pair<string, Client*> &p)
		{ return p.second->access_token == access_token; }) == user_id_clients.end()) {
		result.err = PERMISSION_DENIED;
		cout << "DENY (" << choose_operation(argp->op) << "," << argp->resource << "," << argp->acc_token << ",0)" << endl; 
		return &result;
	}
	
	if (find(resources.begin(), resources.end(), string(argp->resource)) == resources.end()) {
		result.err = RESOURCE_NOT_FOUND;
		cout << "DENY (" << choose_operation(argp->op) << "," << argp->resource << "," << argp->acc_token << ",";
		for (auto it = user_id_clients.begin(); it != user_id_clients.end(); ++it) {
			if (it->second->access_token == access_token) {
				cout << it->second->ttl_access_token << ")" << endl;
			}
		}
		return &result;
	}

	for (auto it = user_id_clients.begin(); it != user_id_clients.end(); ++it) {
		if (it->second->access_token == access_token) {
			if (it->second->ttl_access_token == -1) {
				result.err = TOKEN_EXPIRED;
				cout << "DENY (" << choose_operation(argp->op) << "," << argp->resource << ",,0)" << endl;
				return &result;
			}
		}
	}

	for (auto it = user_id_clients.begin(); it != user_id_clients.end(); ++it) {
		// check if the arpg->operation is in the resource_operations map of the client
		if (find(it->second->resource_operations[argp->resource].begin(),
				it->second->resource_operations[argp->resource].end(), argp->op)
				== it->second->resource_operations[argp->resource].end()) {
			result.err = OPERATION_NOT_PERMITTED;
			cout << "DENY (" << choose_operation(argp->op) << "," << argp->resource << "," << argp->acc_token << ",";
			for (auto it = user_id_clients.begin(); it != user_id_clients.end(); ++it) {
				if (it->second->access_token == access_token) {
					cout << it->second->ttl_access_token << ")" << endl;
				}
			}
			return &result;
		}
	}

	result.err = PERMISSION_GRANTED;

	cout << "PERMIT (" << choose_operation(argp->op) << "," << argp->resource << "," << argp->acc_token << ",";
	for (auto it = user_id_clients.begin(); it != user_id_clients.end(); ++it) {
		if (it->second->access_token == access_token) {
			cout << it->second->ttl_access_token << ")" << endl;
		}
	}

	return &result;
}

int main(int argc, char* argv[]) {

	parse_user_id_file(argv[1]);
	parse_resource_file(argv[2]);
	parse_approvals_file(argv[3]);
	
	if (argc == 5) ttl = atoi(argv[4]);

	// main function in svc file
	register SVCXPRT *transp;

	pmap_unset (RPC_OAUTH_PROG, RPC_OAUTH_VERS);

	transp = svcudp_create(RPC_ANYSOCK);
	if (transp == NULL) {
		fprintf (stderr, "%s", "cannot create udp service.");
		exit(1);
	}
	if (!svc_register(transp, RPC_OAUTH_PROG, RPC_OAUTH_VERS, rpc_oauth_prog_1, IPPROTO_UDP)) {
		fprintf (stderr, "%s", "unable to register (RPC_OAUTH_PROG, RPC_OAUTH_VERS, udp).");
		exit(1);
	}

	transp = svctcp_create(RPC_ANYSOCK, 0, 0);
	if (transp == NULL) {
		fprintf (stderr, "%s", "cannot create tcp service.");
		exit(1);
	}
	if (!svc_register(transp, RPC_OAUTH_PROG, RPC_OAUTH_VERS, rpc_oauth_prog_1, IPPROTO_TCP)) {
		fprintf (stderr, "%s", "unable to register (RPC_OAUTH_PROG, RPC_OAUTH_VERS, tcp).");
		exit(1);
	}

	svc_run ();
	fprintf (stderr, "%s", "svc_run returned");
	exit (1);

	return 0;
}