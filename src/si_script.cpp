#include "si_script.h"

#include "si_control.h"

#include "ryml_std.hpp"
#include "ryml.hpp"

static const char* required_keys[] = {
	"name",
	"duration",
	"events",
};
static const size_t num_required_keys = sizeof(required_keys) / sizeof(required_keys[0]);

si_script::si_script()
	: duration(0.0f)
{
}

bool si_script::parse(std::vector<char>& buf)
{
	// Parse the YAML data from our string buffer (this can call abort() if data is malformed)
	ryml::substr view = c4::basic_substring<char>(buf.data(), buf.size());
	ryml::Tree tree = ryml::parse(view);
	ryml::NodeRef root = tree.rootref();

	// The root (i.e. the YAML document itself) must be a map
	if (!root.is_map())
	{
		printf("ERROR: Parsed YAML is not a map\n");
		return false;
	}

	// Preemptively check for all required top-level keys
	for (size_t key_index = 0; key_index < num_required_keys; key_index++)
	{
		const char* key = required_keys[key_index];
		c4::csubstr key_view(key, strlen(key));
		if (!root.has_child(key_view))
		{
			printf("ERROR: YAML is missing required key '%s'\n", key);
			return false;
		}
	}

	// Parse the name of the script
	ryml::NodeRef name_node = root["name"];
	if (!name_node.is_keyval())
	{
		printf("ERROR: YAML node 'name' is not a keyval\n");
		return false;
	}
	name_node >> name;

	// Parse the duration of the sequence
	ryml::NodeRef duration_node = root["duration"];
	if (!duration_node.is_keyval() || !duration_node.val().is_number())
	{
		printf("ERROR: YAML node 'duration' is not a number\n");
		return false;
	}
	duration_node >> duration;
	if (duration <= 0.0f)
	{
		printf("ERROR: Script duration %0.2f is invalid; duration must be positive\n", duration);
		return false;
	}

	// Process the 'events' sequence to parse each input event
	ryml::NodeRef events_node = root["events"];
	if (!events_node.is_seq())
	{
		printf("ERROR: YAML node 'events' is not a sequence\n");
		return false;
	}
	const size_t num_event_nodes = events_node.num_children();

	// Each node in the YAML's 'events' list can have multiple control events: loop through initially to count how many unique events we need to store
	size_t num_events = 0;
	for (size_t event_node_index = 0; event_node_index < num_event_nodes; event_node_index++)
	{
		ryml::NodeRef event_node = events_node[event_node_index];
		if (!event_node.is_map())
		{
			printf("ERROR: YAML 'events' entry at index %zu is not a map\n", event_node_index);
			return false;
		}

		if (!event_node.has_child("at"))
		{
			printf("ERROR: YAML 'events' entry at index %zu is missing required key 'at'\n", event_node_index);
			return false;
		}

		const size_t num_controls_at_event_node = event_node.num_children() - 1;
		num_events++;
	}
	events.resize(num_events);

	// Run a second loop to parse each si_event from its YAML definition
	float prev_event_time = 0.0f;
	size_t event_index = 0;
	for (size_t event_node_index = 0; event_node_index < num_event_nodes; event_node_index++)
	{
		// Get the 'at' node, which we already know exists, and require that it's a number
		ryml::NodeRef event_node = events_node[event_node_index];
		ryml::NodeRef at_node = event_node["at"];
		if (!at_node.is_keyval() || !at_node.val().is_number())
		{
			printf("ERROR: YAML 'events' entry at index %zu: 'at' value is not a number\n", event_node_index);
			return false;
		}

		// Read the timestamp for this event and validate that it's in sequence
		float event_time;
		at_node >> event_time;
		if (event_time < prev_event_time)
		{
			printf("ERROR: YAML 'events' entry at index %zu: 'at' timestamp %0.2f is less than previous %0.2f; events should be in order\n", event_node_index, event_time, prev_event_time);
			return false;
		}
		if (event_time > duration)
		{
			printf("ERROR: YAML 'events' entry at index %zu: 'at' timestamp %0.2f exceeds sequence duration of %0.2f\n", event_node_index, event_time, duration);
			return false;
		}

		// Loop through every other key/value pair in this event map
		for (ryml::NodeRef child = event_node.first_child(); child.valid(); child = child.next_sibling())
		{
			// Skip the 'at' key, we've already dealt with it preemptively
			assert(child.has_key());
			if (child.key().compare("at") == 0)
			{
				continue;
			}

			// All other keys should be valid control names, mapped to the desired state of that control at the given time
			si_control control;
			const char* key_name_str = child.key().data();
			const size_t key_name_len = child.key().len;
			if (!si_control_parse(key_name_str, key_name_len, control))
			{
				printf("ERROR: YAML 'events' entry at index %zu: key '%.*s' is not a valid control name\n", event_node_index, static_cast<int>(key_name_len), key_name_str);
				return false;
			}

			// Defer to si_event for parsing the actual state value
			assert(event_index < events.size());
			si_event& event = events[event_index];
			if (!event.parse(event_time, duration, control, child))
			{
				printf("ERROR: YAML 'events' entry at index %zu: failed to parse desired state for control '%.*s'\n", event_node_index, static_cast<int>(key_name_len), key_name_str);
				return false;
			}
			event_index++;
		}
	}
	return true;
}
