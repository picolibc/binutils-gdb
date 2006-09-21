// layout.h -- lay out output file sections for gold  -*- C++ -*-

#ifndef GOLD_LAYOUT_H
#define GOLD_LAYOUT_H

#include <list>
#include <string>
#include <utility>
#include <vector>

#include "options.h"
#include "workqueue.h"
#include "object.h"
#include "stringpool.h"

namespace gold
{

class Output_section;
class Output_segment;

// This Task handles mapping the input sections to output sections and
// laying them out in memory.

class Layout_task : public Task
{
 public:
  // OPTIONS is the command line options, INPUT_OBJECTS is the list of
  // input objects, THIS_BLOCKER is a token which blocks this task
  // from executing until all the input symbols have been read.
  Layout_task(const General_options& options, const Object_list* input_objects,
	      Task_token* this_blocker)
    : options_(options), input_objects_(input_objects),
      this_blocker_(this_blocker)
  { }

  ~Layout_task();

  // The standard Task methods.

  Is_runnable_type
  is_runnable(Workqueue*);

  Task_locker*
  locks(Workqueue*);

  void
  run(Workqueue*);

 private:
  Layout_task(const Layout_task&);
  Layout_task& operator=(const Layout_task&);

  const General_options& options_;
  const Object_list* input_objects_;
  Task_token* this_blocker_;
};

// This class handles the details of laying out input sections.

class Layout
{
 public:
  Layout(const General_options& options)
    : options_(options), namepool_(), signatures_(),
      section_name_map_(), segment_list_()
  { }

  // Given an input section named NAME with data in SHDR from the
  // object file OBJECT, return the output section where this input
  // section should go.  Set *OFFSET to the offset within the output
  // section.
  template<int size, bool big_endian>
  Output_section*
  layout(Object *object, const char* name,
	 const elfcpp::Shdr<size, big_endian>& shdr, off_t* offset);

  // Return whether a section is a .gnu.linkonce section, given the
  // section name.
  static inline bool
  is_linkonce(const char* name)
  { return strncmp(name, ".gnu.linkonce", sizeof(".gnu.linkonce") - 1) == 0; }

  // Record the signature of a comdat section, and return whether to
  // include it in the link.  The GROUP parameter is true for a
  // section group signature, false for a signature derived from a
  // .gnu.linkonce section.
  bool
  add_comdat(const char*, bool group);

 private:
  Layout(const Layout&);
  Layout& operator=(const Layout&);

  // Mapping from .gnu.linkonce section names to output section names.
  struct Linkonce_mapping
  {
    const char* from;
    int fromlen;
    const char* to;
  };
  static const Linkonce_mapping linkonce_mapping[];
  static const int linkonce_mapping_count;

  // Return whether to include this section in the link.
  template<int size, bool big_endian>
  bool
  include_section(Object* object, const char* name,
		  const elfcpp::Shdr<size, big_endian>&);

  // Return the output section name to use for a linkonce section
  // name.
  static const char*
  linkonce_output_name(const char* name);

  // Create a new Output_section.
  Output_section*
  make_output_section(const char* name, elfcpp::Elf_Word type,
		      elfcpp::Elf_Xword flags);

  // Return whether SEG1 comes before SEG2 in the output file.
  static bool
  Layout::segment_precedes(const Output_segment* seg1,
			   const Output_segment* seg2);

  // Map from section flags to segment flags.
  static elfcpp::Elf_Word
  section_flags_to_segment(elfcpp::Elf_Xword flags);

  // A mapping used for group signatures.
  typedef Unordered_map<std::string, bool> Signatures;

  // Mapping from input section name/type/flags to output section.  We
  // use canonicalized strings here.

  typedef std::pair<const char*,
		    std::pair<elfcpp::Elf_Word, elfcpp::Elf_Xword> > Key;

  struct Hash_key
  {
    size_t
    operator()(const Key& k) const;
  };

  typedef Unordered_map<Key, Output_section*, Hash_key> Section_name_map;

  // A comparison class for segments.

  struct Compare_segments
  {
    bool
    operator()(const Output_segment* seg1, const Output_segment* seg2)
    { return Layout::segment_precedes(seg1, seg2); }
  };

  // The list of segments.

  typedef std::list<Output_segment*> Segment_list;

  // The list of sections not attached to a segment.

  typedef std::list<Output_section*> Section_list;

  // A reference to the options on the command line.
  const General_options& options_;
  // The output section names.
  Stringpool namepool_;
  // The list of group sections and linkonce sections which we have seen.
  Signatures signatures_;
  // The mapping from input section name/type/flags to output sections.
  Section_name_map section_name_map_;
  // The list of output segments.
  Segment_list segment_list_;
  // The list of output sections which are not attached to any output
  // segment.
  Section_list section_list_;
};

} // End namespace gold.

#endif // !defined(GOLD_LAYOUT_H)
