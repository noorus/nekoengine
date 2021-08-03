#include "stdafx.h"
#include "loader.h"
#include "utilities.h"
#include "lodepng.h"
#include "gfx_types.h"
#include "renderer.h"
#include "console.h"

namespace neko::loaders {

  UnityYamlNode* findYamlNode( UnityYamlNode& root, const utf8String& name )
  {
    if ( root.name == name )
      return &root;
    for ( auto& child : root.children )
    {
      auto ret = findYamlNode( *child, name );
      if ( ret )
        return ret;
    }
    return nullptr;
  }

  void dumpUnityYaml( UnityYamlNode& node, size_t level )
  {
    utf8String prep;
    for ( int i = 0; i < level; i++ )
      prep.append( "  " );
    utf8String attrs;
    for ( const auto& a : node.attribs )
      attrs.append( ", " + a.first );
    Locator::console().printf( Console::srcLoader, "%syaml: %i node %s attribs %i%s", prep.c_str(), node.indent, node.name.c_str(), node.attribs.size(), attrs.c_str() );
    for ( auto& child : node.children )
      dumpUnityYaml( *child, level + 1 );
  }

  void createAnimation( UnityYamlNode& root )
  {
    ozz::animation::offline::RawAnimation animation;
    auto settings = findYamlNode( root, "m_AnimationClipSettings" );
    if ( !settings )
      NEKO_EXCEPT( "No m_AnimationClipSettings node" );
    animation.duration = static_cast<float>( ::atof( settings->attribs["m_StopTime"].c_str() ) );
    //
  }

  void loadUnityYaml( const vector<uint8_t>& data )
  {
    auto parseLine = []( const utf8String line, UnityYamlNode*& node, map<size_t, UnityYamlNode*>& lastindentmap, int& previndent ) -> void
    {
      auto first = line.find_first_not_of( ' ' );
      if ( first == utf8String::npos || line[first] == '%' )
        return;
      auto next = line.find_first_not_of( '-', first );
      if ( next == utf8String::npos || ( next - first ) > 2 )
        return;
      auto dot = line.find_first_of( ':', first );
      if ( dot == utf8String::npos )
        NEKO_EXCEPT( "Don't know how to deal with missing :" );
      auto newsibling = false;
      if ( line[first] == '-' && line[first + 1] == ' ' )
      {
        first += 2;
        newsibling = true;
      }
      auto indent = ( first / 2 );
      //if ( line.find( "pptrCurveMapping" ) != line.npos )
      //  DebugBreak();
      if ( indent < node->indent )
      {
        node = lastindentmap[indent];
        while ( node->name.empty() )
        {
          node = node->parent;
        }
      }
      else if ( indent == ( previndent - 1 ) && !node->parent->name.empty() && node->children.empty() )
      {
        node = node->parent;
        lastindentmap[indent] = node;
      }
      previndent = indent;
      auto name = line.substr( first, dot - first );
      if ( newsibling )
      {
        auto parent = node;
        if ( node->parent->name.empty() || indent == node->indent )
          parent = node->parent;
        auto newnode = new UnityYamlNode( parent, indent );
        parent->children.emplace_back( newnode );
        node = newnode;
      }
      if ( dot == line.size() - 1 )
      {
        if ( !newsibling )
        {
          auto parent = node;
          auto newnode = new UnityYamlNode( parent, indent );
          parent->children.emplace_back( newnode );
          node = newnode;
        }
        node->name = name;
      }
      else
      {
        auto value = line.substr( dot + 2 );
        node->attribs[name] = value;
        lastindentmap[indent] = node;
      }
    };

    size_t p = 0, i = 0;
    UnityYamlNode root( nullptr, 0 );
    root.isRoot = true;
    map<size_t, UnityYamlNode*> lastindentmap;
    auto curnode = &root;
    int prev = 0;
    while ( i < data.size() )
    {
      if ( data[i] == '\r' || data[i] == '\n' )
      {
        if ( i - p > 1 )
        {
          utf8String line( i - p, ' ' );
          memcpy( line.data(), &data[p], i - p );
          parseLine( line, curnode, lastindentmap, prev );
        }
        if ( i < ( data.size() - 1 ) && data[i + 1] == '\n' )
          i++;
        p = i + 1;
      }
      i++;
    }

    dumpUnityYaml( root );
  }

}