#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <iostream>
#include <set>

typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::connection_hdl connection_hdl;

class chat_server {
public:
    chat_server() {
        ws_server.init_asio();

        // Configura os handlers
        ws_server.set_open_handler(std::bind(&chat_server::on_open, this, std::placeholders::_1));
        ws_server.set_close_handler(std::bind(&chat_server::on_close, this, std::placeholders::_1));
        ws_server.set_message_handler(std::bind(&chat_server::on_message, this, std::placeholders::_1, std::placeholders::_2));
    }

    void run(uint16_t port) {
        ws_server.listen(port);
        ws_server.start_accept();
        std::cout << "Servidor WebSocket ouvindo na porta " << port << "..." << std::endl;
        ws_server.run();
    }

private:
    server ws_server;
    std::set<connection_hdl, std::owner_less<connection_hdl>> connections;

    void on_open(connection_hdl hdl) {
        connections.insert(hdl);
        std::cout << "Novo cliente conectado." << std::endl;
    }

    void on_close(connection_hdl hdl) {
        connections.erase(hdl);
        std::cout << "Cliente desconectado." << std::endl;
    }

    void on_message(connection_hdl hdl, server::message_ptr msg) {
        std::string mensagem = msg->get_payload();
        std::cout << "Mensagem recebida: " << mensagem << std::endl;

        // Encaminha a mensagem para todos os clientes conectados
        for (const auto& conn : connections) {
            if (conn.lock() != hdl.lock()) { // Ignora o remetente
                ws_server.send(conn, mensagem, websocketpp::frame::opcode::text);
            }
        }
    }
};

int main() {
    chat_server servidor;
    servidor.run(9890);
    return 0;
}
