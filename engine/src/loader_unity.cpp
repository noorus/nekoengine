#include "stdafx.h"
#include "loader.h"
#include "utilities.h"
#include "lodepng.h"
#include "gfx_types.h"
#include "renderer.h"
#include "console.h"

namespace neko::loaders {

  UnityYamlNode* findUnityYamlNode( UnityYamlNode& root, const utf8String& name )
  {
    if ( root.name == name )
      return &root;
    for ( auto& child : root.children )
    {
      auto ret = findUnityYamlNode( *child, name );
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

  vec3 extract_v3( const utf8String& blob )
  {
    vec3 out;
    std::regex e( "(x|y|z|w):" );
    auto fixd = std::regex_replace( blob, e, R"("$1":)" );
    auto json = nlohmann::json::parse( fixd );
    out.x = static_cast<Real>( json["x"].get<double>() );
    out.y = static_cast<Real>( json["y"].get<double>() );
    out.z = static_cast<Real>( json["z"].get<double>() );
    return move( out );
  }

  quaternion extract_q( const utf8String& blob )
  {
    quaternion out;
    std::regex e( "(x|y|z|w):" );
    auto fixd = std::regex_replace( blob, e, R"("$1":)" );
    auto json = nlohmann::json::parse( fixd );
    out.x = static_cast<Real>( json["x"].get<double>() );
    out.y = static_cast<Real>( json["y"].get<double>() );
    out.z = static_cast<Real>( json["z"].get<double>() );
    out.w = static_cast<Real>( json["w"].get<double>() );
    return move( out );
  }

  ozz::unique_ptr<ozz::animation::Animation> buildAnimationFromUnityYaml( UnityYamlNode& root )
  {
    ozz::animation::offline::RawAnimation animation;
    size_t boneIndex = 0;
    map<utf8String, size_t> boneMap;
    auto clip = findUnityYamlNode( root, "AnimationClip" );
    auto settings = findUnityYamlNode( *clip, "m_AnimationClipSettings" );
    auto translations = findUnityYamlNode( *clip, "m_PositionCurves" );
    auto rotations = findUnityYamlNode( *clip, "m_RotationCurves" );
    if ( !clip || !settings || !translations || !rotations )
      NEKO_EXCEPT( "Required node(s) missing" );

    animation.duration = static_cast<float>( ::atof( settings->attribs["m_StopTime"].c_str() ) );
    animation.name = clip->attribs["m_Name"];

    for ( auto& node : translations->children )
      if ( node->name == "curve" && !node->children.empty() )
        if ( node->children[0]->name == "m_Curve" )
        {
          if ( boneMap.find( node->attribs["path"] ) == boneMap.end() )
            boneMap[node->attribs["path"]] = boneIndex++;
          if ( boneMap.size() > animation.tracks.size() )
            animation.tracks.resize( boneMap.size() );
          auto& track = animation.tracks[boneMap[node->attribs["path"]]];
          for ( auto pt : node->children[0]->children )
          {
            auto pos = extract_v3( pt->attribs["value"] );
            ozz::animation::offline::RawAnimation::TranslationKey key;
            key.time = static_cast<float>( ::atof( pt->attribs["time"].c_str() ) );
            key.value = ozz::math::Float3( pos.x, pos.y, pos.z );
            track.translations.push_back( move( key ) );
          }
        }

    for ( auto& node : rotations->children )
      if ( node->name == "curve" && !node->children.empty() )
        if ( node->children[0]->name == "m_Curve" )
        {
          if ( boneMap.find( node->attribs["path"] ) == boneMap.end() )
            boneMap[node->attribs["path"]] = boneIndex++;
          if ( boneMap.size() > animation.tracks.size() )
            animation.tracks.resize( boneMap.size() );
          auto& track = animation.tracks[boneMap[node->attribs["path"]]];
          for ( auto pt : node->children[0]->children )
          {
            auto rot = extract_q( pt->attribs["value"] );
            ozz::animation::offline::RawAnimation::RotationKey key;
            key.time = static_cast<float>( ::atof( pt->attribs["time"].c_str() ) );
            key.value = ozz::math::Quaternion( rot.x, rot.y, rot.z, rot.w );
            track.rotations.push_back( move( key ) );
          }
        }

    if ( !animation.Validate() )
      NEKO_EXCEPT( "Ozz animation validate failed" );

    ozz::animation::offline::AnimationBuilder builder;
    return move( builder( animation ) );
  }

  ozz::unique_ptr<ozz::animation::Animation> loadUnityYaml( const vector<uint8_t>& data )
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

    return move( buildAnimationFromUnityYaml( root ) );
  }

}