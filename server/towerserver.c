/**
 *
 */

#include <stdio.h>
#include <string.h>
#include <jansson.h>
#include "mongoose.h"

char *

static void send_message(struct mg_connection *conn, char *msg, size_t msg_len);

/**
 * Sends the list animatinos to the client.
 */
static void tl_send_animationlist(struct mg_connection *conn) {
  json_t *root;
  json_t *array;
  char *msg;

  //root = json_pack("{s:s,s:[{s:s,s:s}]}", "type", "animationlist", "animations", "title", "Test Animation", "musicfile", "animations/testanimation/music.wav"), 
  root = json_object();
  json_object_set(root, "type", json_string("animationlist"));
  array = json_array(); 

  // Goal: Each folder in the specified animations folder holds an animation and it's music. Enumerate all the animations their music files. 
   

  json_object_set(root, "animations", array);

  

  msg = json_dumps(root, 0);
  if (msg == NULL) {
    printf("An error occured while dumping the json string\n");
  }

  send_message(conn, msg, strlen(msg));

  json_decref(root); 
}

/**
 * Send a pong message to the the client. 
 */
static void tl_send_pong(struct mg_connection *conn) {
  json_t *root;
  char *msg;

  root = json_pack("{s:s}", "type", "pong");
  msg = json_dumps(root, 0);

  send_message(conn, msg, strlen(msg));
 
  json_decref(root); 
}

/** 
 * Constructs a JSON object representing an error object and sends it to the client.
 */
static void tl_send_error(const char *errormsg, struct mg_connection *conn) {
  json_t *root;
  char *msg;

  root = json_pack("{s:s, s:s}", "type", "error", "message", errormsg);
  msg = json_dumps(root, 0);

  send_message(conn, msg, strlen(msg));
 
  json_decref(root); 
}

/** 
 * Sends a message to the client.
 */
static void send_message(struct mg_connection *conn, char *msg, size_t msg_len) {
  unsigned char reply[200];
  int i;

  if (msg_len > 125) {
    msg_len = 125;
  }

  reply[0] = 0x81;  // text, FIN set
  reply[1] = msg_len;

  for (i = 0; i < msg_len; i++) {
    reply[i+2] = msg[i];
  }

  mg_write(conn, reply, 2 + msg_len);
}

/**
 * Callback function from mongoose when a websocket connection is made.
 * Sends a message back to the client saying that the server is ready. 
 * 
 */
static void websocket_ready_handler(struct mg_connection *conn) {
  /*
  unsigned char buf[40];
  buf[0] = 0x81;
  buf[1] = snprintf((char *) buf + 2, sizeof(buf) - 2, "%s", "server ready");
  mg_write(conn, buf, 2 + buf[1]);
  */
}

/**
 * Callback function from mongoose when a websocket has data. 
 * This function will interpret the message and take appropriate actions.
 * 
 * Arguments:
 *   flags: first byte of websocket frame, see websocket RFC,
 *          http://tools.ietf.org/html/rfc6455, section 5.2
 *   data, data_len: payload data. Mask, if any, is already applied.
 */
static int websocket_data_handler(struct mg_connection *conn, int flags, char *data, size_t data_len) {
  printf("Recieved: [%.*s]\n", (int) data_len, data);
  
  // A variable to hold any errors from jansson
  json_error_t error;
  // The root node in the decoded JSON object
  json_t *root;

  // Decode the recieved text
  root = json_loadb(data, data_len, 0, &error);

  // Check if the decoding went ok...
  if (root == NULL) {
    fprintf(stderr, "error: on line %d, column %d: %s\n", error.line, error.column, error.text);
    send_message(conn, "Error decoded JSON string", 25);
  } else {
    // Now we have a decoded JSON object.
    // Check if the root is an object
    if (json_is_object(root) == 0) {
      send_message(conn, "Message must be a JSON object", 30);
    } else {
      // The object should have type property
      json_t *type = json_object_get(root, "type");
      if (json_is_string(type) == 0) {
        send_message(conn, "Message type missing", 21);
      } else {
        // Now handle the message!
        const char *msgtype = json_string_value(type);

        if (strncmp(msgtype, "ping", 5) == 0) {
          tl_send_pong(conn);
        } else if (strncmp(msgtype, "getanimations", 13) == 0) {
          tl_send_animationlist(conn);
        } else {
          tl_send_error("Unrecognized Command", conn);
        }
      }
      
      json_decref(root);
    }
  }

  // Returning zero means stoping websocket conversation.
  return 1;
}

int main(void) {
  struct mg_context *ctx;
  struct mg_callbacks callbacks;
  const char *options[] = {
    "listening_ports", "8080",
    "document_root", "www",
    NULL
  };

  // Initialize callbacks to null
  memset(&callbacks, 0, sizeof(callbacks));
  // Handle the ready callback
  callbacks.websocket_ready = websocket_ready_handler;
  // Handle the data callback
  callbacks.websocket_data = websocket_data_handler;
  // Start the server
  ctx = mg_start(&callbacks, NULL, options);
  // Wait until user hits "enter"
  getchar();  
  // Stop the server
  mg_stop(ctx);

  return 0;
}
