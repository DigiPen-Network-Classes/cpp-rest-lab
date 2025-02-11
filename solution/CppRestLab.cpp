#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS 1
#include <iostream>
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <cpprest/uri_builder.h>
std::string stored_username = "user1";
std::string stored_password = "password1xx";
std::string stored_session;

int main() {
    web::http::client::http_client client(U("http://localhost:25000"));
    client.request(web::http::methods::GET, U("/lab"));

    auto login_data = web::json::value::parse("{}");
    login_data[U("username")] = web::json::value(utility::conversions::to_string_t(stored_username));
    login_data[U("password")] = web::json::value(utility::conversions::to_string_t(stored_password));
    auto req = client
        // post to login with our username & password
        .request(web::http::methods::POST, U("/lab/login"), login_data)
        // take the response and extract the response body
        .then([](web::http::http_response login_response) {
            std::cout << login_response.status_code() << std::endl << std::flush;
            if (login_response.status_code() != web::http::status_codes::OK) {
                throw std::exception("Lab service login failed.");
            }
            return login_response.extract_json();
        })
        // with the response body (our session_data object),
        // extract the session value as a string,
        // create a new URI for /lab/users?session=(session_string)
        // then go GET that uri.
        .then([=](web::json::value session_data) mutable {
            auto session = session_data[U("session")].as_string();
            stored_session = utility::conversions::to_utf8string(session);
            std::cout << stored_session << std::endl << std::flush;
            web::http::uri_builder uri_builder(U("/lab/users"));
            uri_builder.append_query(U("session"), session);
            return client.request(web::http::methods::GET, uri_builder.to_string());
        })

        // get the status code and extract the response body (just as a string for now)
        .then([](web::http::http_response get_users_response) {
            std::cout << get_users_response.status_code() << std::endl << std::flush;
            if (get_users_response.status_code() != web::http::status_codes::OK) {
                throw std::exception("Lab service authenticated-GET-users failed.");
            }
            return get_users_response.extract_utf8string();
        })
        .then([](std::string users_str) {
            std::cout << users_str << std::endl << std::flush;
        });

    try {
        req.wait();
    }
    catch (const std::exception& e) {
        std::cout << "Error - " << e.what() << std::endl << std::flush;
    }
    return 0;
}
