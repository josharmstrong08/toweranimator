/**
 *
 */

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <jansson.h>
#include "mongoose.h"

static void send_message(struct mg_connection *conn, char *msg, size_t msg_len);
static void tl_send_error(const char *errormsg, struct mg_connection *conn);
static void tl_send_animationlist(struct mg_connection *conn);

static char rootdir[512] = "/home/josh/Documents/Tower Lights Animations/";

/**
 * Sends the list animatinos to the client.
 */
static void tl_send_animationlist(struct mg_connection *conn) {
  json_t *root;
  json_t *array;
  char *msg;

  root = json_object();
  json_object_set_new(root, "type", json_string("animationlist"));
  array = json_array(); 

  // Goal: Each folder in the specified animations folder holds an animation and it's music. 
  // Enumerate all the animations their music files
  DIR *dirp = opendir(rootdir);
  struct dirent *dp;

  // Check if the directory exists
  if (dirp == NULL) {
    printf("Could not animation directory '%s'.\n", rootdir);
    tl_send_error("Could not open root animation directory", conn);
    return;
  }

  // Iterate over the directories
  while ((dp = readdir(dirp)) != NULL) {
    if (dp->d_type == DT_DIR) {
      if (strncmp(dp->d_name, ".", 1) != 0 &&
          strncmp(dp->d_name, "..", 2) != 0) {
        json_t *animation = json_object();       
        json_object_set_new(animation, "title", json_string(dp->d_name));
  
        printf("Animation title: %s\n", dp->d_name); 

        char newdir[100]; 
        strncpy(newdir, rootdir, 100);
        strcat(strcat(newdir, "/"), dp->d_name);

        DIR *dirp2 = opendir(newdir);
        struct dirent *dp2;
        int foundtan = 0;
        int foundwav = 0;

        // Look for one wav file and one tan file
        while ((dp2 = readdir(dirp2)) != NULL) {
          if (dp2->d_type == DT_REG) {
            if (strncmp((dp2->d_name + strlen(dp2->d_name) - 4), ".wav", 4) == 0) {
              if (foundwav == 1) {
                printf("  Error: multiple wav files in a directory\n");
              } else {
                // printf("  wav file: %s\n", dp2->d_name);
                json_object_set_new(animation, "music", json_string(dp2->d_name));
              }
              foundwav = 1;
            } else if (strncmp((dp2->d_name + strlen(dp2->d_name) - 4), ".tan", 4) == 0) {
              if (foundtan == 1) {
                printf("  Error: multiple tan files in a directory\n");
              } else {
                // printf("  tan file: %s\n", dp2->d_name);
                json_object_set_new(animation, "tan", json_string(dp2->d_name));
              }
              foundtan = 1;
            }
          }
        }

        json_array_append_new(array, animation);
        closedir(dirp2);
      }

    }
  }

  closedir(dirp);

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
  unsigned char *reply;
  int i;
  int offset = 0;

  reply = (unsigned char *)malloc(sizeof(unsigned char) * msg_len);
  reply[0] = 0x81;  // text, FIN set

  if (msg_len <= 125) {
    reply[1] = msg_len;
    offset = 2;
  } else if (msg_len <= 65535) {
    reply[1] = 126;
    reply[2] = (msg_len & 0xff00) >> 8;
    reply[3] = msg_len & 0x00ff;
    offset = 4;
  } else if (msg_len <= 0xffffffffffffffff) {
    reply[1] = 127;
    unsigned long long mask =  0xff00000000000000;
    for (i = 0; i < 8; i++) {
      reply[i+2] = (msg_len & mask) >> ((7-i) * 8);
      mask = mask >> 16;
    }
    offset = 10;
  } else {
    printf("Error: message is too large to send.\n");
    tl_send_error("Error sending message: payload too large", conn);
  }

  for (i = 0; i < msg_len; i++) {
    reply[i+offset] = msg[i];
  }

  mg_write(conn, reply, offset + msg_len);
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

