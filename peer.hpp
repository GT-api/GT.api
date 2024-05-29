
class peer {

};

#define getpeer std::unique_ptr<peer>{static_cast<peer*>(event.peer->data)}
