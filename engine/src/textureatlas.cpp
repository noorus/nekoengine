#include "pch.h"
#include "locator.h"
#include "neko_exception.h"
#include "textureatlas.h"

namespace neko {

  TextureAtlas::TextureAtlas( const size_t width, const size_t height, const size_t depth )
  {
    assert( depth == 1 || depth == 3 || depth == 4 );

    nodes_.emplace_back( 1, 1, width - 2 );

    used_ = 0;
    width_ = width;
    height_ = height;
    depth_ = depth;
    id_ = 0;

    data_.resize( width * height * depth );
    memset( data_.data(), 0, data_.size() );
  }

  void TextureAtlas::setRegion( const size_t x, const size_t y, const size_t width, const size_t height, const uint8_t* data, const size_t stride )
  {
    assert( x > 0 && y > 0 && ( x < ( width_ - 1 ) ) && ( y < ( height_ - 1 ) ) );
    assert( ( x + width ) <= ( width_ - 1 ) && ( y + height ) <= ( height_ - 1 ) );

    assert( height == 0 || ( data && width > 0 ) );

    auto depth = depth_;
    size_t charsize = sizeof( uint8_t );
    for ( size_t i = 0; i < height; ++i )
    {
      memcpy(
        data_.data() + ( ( y + i ) * width_ + x ) * charsize * depth,
        data + ( i * stride ) * charsize, width * charsize * depth );
    }
  }

  int TextureAtlas::fit( const size_t index, const size_t width, const size_t height )
  {
    auto node = &nodes_[index];
    auto x = node->x;
    auto y = node->y;
    auto width_left = (int64_t)width;
    auto i = index;

    if ( ( x + width ) > ( width_ - 1 ) )
      return -1;

    while ( width_left > 0 )
    {
      node = &nodes_[i];
      if ( node->y > y )
        y = node->y;
      if ( ( y + height ) > ( height_ - 1 ) )
        return -1;
      width_left -= node->z;
      ++i;
    }

    return (int)y;
  }

  void TextureAtlas::merge()
  {
    size_t i = 0;
    for ( i = 0; i < nodes_.size() - 1; ++i )
    {
      auto node = &nodes_[i];
      auto next = &nodes_[i + 1];
      if ( node->y == next->y )
      {
        node->z += next->z;
        nodes_.erase( nodes_.begin() + ( i + 1 ) );
        --i;
      }
    }
  }

  vec4i TextureAtlas::getRegion( const size_t width, const size_t height )
  {
    int best_index = -1;
    size_t best_height = numeric_limits<size_t>::max();
    size_t best_width = numeric_limits<size_t>::max();

    vec4i region;

    for ( size_t i = 0; i < nodes_.size(); ++i )
    {
      auto y = fit( i, width, height );
      if ( y >= 0 )
      {
        auto node = &nodes_[i];
        if ( ( ( y + height ) < best_height ) || ( ( ( y + height ) == best_height ) && ( node->z > 0 && (size_t)node->z < best_width ) ) )
        {
          best_height = ( y + height );
          best_index = (int)i;
          best_width = node->z;
          region.x = node->x;
          region.y = y;
        }
      }
    }

    if ( best_index == -1 )
    {
      region.x = -1;
      region.y = -1;
      region.z = 0;
      region.w = 0;
      return move( region );
    }

    vec3i node( region.x, region.y + height, width );
    nodes_.insert( nodes_.begin() + best_index, node );

    size_t i;
    for ( i = (size_t)best_index + 1; i < nodes_.size(); ++i )
    {
      auto node = &nodes_[i];
      auto prev = &nodes_[i - 1];
      if ( node->x < ( prev->x + prev->z ) )
      {
        auto shrink = prev->x + prev->z - node->x;
        node->x += shrink;
        node->z -= shrink;
        if ( node->z <= 0 )
        {
          nodes_.erase( nodes_.begin() + i );
          --i;
        }
        else
          break;
      }
      else
        break;
    }

    merge();
    used_ += ( width * height );
    return move( region );
  }

  void TextureAtlas::clear()
  {
    vec3i node( 1, 1, 1 );
    nodes_.clear();
    used_ = 0;
    node.z = width_ - 2;
    nodes_.push_back( move( node ) );
    memset( data_.data(), 0, width_ * height_ * depth_ );
  }

}