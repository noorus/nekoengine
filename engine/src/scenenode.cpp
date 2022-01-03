#include "pch.h"
#include "renderer.h"

namespace neko {

  void SceneNode::update( bool children, bool parentChanged )
  {
    if ( needParentUpdate_ || parentChanged )
      updateFromParent();
    if ( children && ( needChildUpdate_ || parentChanged ) )
    {
      for ( auto& child : children_ )
        child->update( true, true );
      needChildUpdate_ = false;
    }
  }

  void SceneNode::needUpdate()
  {
    needParentUpdate_ = true;
    needChildUpdate_ = true;
    cachedOutOfDate_ = true;
  }

  void SceneNode::setTranslate( const vec3& position )
  {
    translate_ = position;
    needUpdate();
  }

  void SceneNode::setScale( const vec3& scale )
  {
    scale_ = scale;
    needUpdate();
  }

  void SceneNode::setRotate( const quaternion& rotate )
  {
    rotate_ = math::normalize( rotate );
    needUpdate();
  }

  void SceneNode::translate( const vec3& position )
  {
    translate_ += position;
    needUpdate();
  }

  void SceneNode::scale( const vec3& scale )
  {
    scale_ *= scale;
    needUpdate();
  }

  void SceneNode::rotate( const quaternion& rotation )
  {
    rotate_ = math::normalize( rotation * rotate_ );
    needUpdate();
  }

  const vec3& SceneNode::getDerivedTranslate() const
  {
    if ( needParentUpdate_ )
      updateFromParent();
    return derivedTranslate_;
  }

  const vec3& SceneNode::getDerivedScale() const
  {
    if ( needParentUpdate_ )
      updateFromParent();
    return derivedScale_;
  }

  const quaternion& SceneNode::getDerivedRotate() const
  {
    if ( needParentUpdate_ )
      updateFromParent();
    return derivedRotate_;
  }

  const mat4& SceneNode::getFullTransform() const
  {
    if ( cachedOutOfDate_ )
    {
      cachedTransform_ = mat4( numbers::one );
      cachedTransform_ = glm::scale( cachedTransform_, getDerivedScale() );
      cachedTransform_ *= glm::toMat4( getDerivedRotate() );
      cachedTransform_ = glm::translate( cachedTransform_, getDerivedTranslate() );
      // cachedTransform_.makeTransform( getDerivedTranslate(), getDerivedScale(), getDerivedRotate() );
      cachedOutOfDate_ = false;
    }
    return cachedTransform_;
  }

  vec3 SceneNode::convertLocalToWorldPosition( const vec3& localPosition )
  {
    if ( needParentUpdate_ )
      updateFromParent();
    return mat3( getFullTransform() ) * localPosition;
  }

  vec3 SceneNode::convertWorldToLocalPosition( const vec3& worldPosition )
  {
    if ( needParentUpdate_ )
      updateFromParent();
    return math::inverse( derivedRotate_ ) * ( worldPosition - derivedTranslate_ ) / derivedScale_;
  }

  quaternion SceneNode::convertLocalToWorldOrientation( const quaternion& localOrientation )
  {
    if ( needParentUpdate_ )
      updateFromParent();
    return derivedRotate_ * localOrientation;
  }

  quaternion SceneNode::convertWorldToLocalOrientation( const quaternion& worldOrientation )
  {
    if ( needParentUpdate_ )
      updateFromParent();
    return math::inverse( derivedRotate_ ) * worldOrientation;
  }

  void SceneNode::updateFromParent() const
  {
    cachedOutOfDate_ = true;
    needParentUpdate_ = false;
    if ( parent_ )
    {
      const auto& prot = parent_->getDerivedRotate();
      derivedRotate_ = ( inheritOrientation_ ? prot * rotate_ : rotate_ );
      const auto& pscale = parent_->getDerivedScale();
      derivedScale_ = ( inheritScale_ ? pscale * scale_ : scale_ );
      derivedTranslate_ = prot * ( pscale * translate_ );
      derivedTranslate_ += parent_->getDerivedTranslate();
      return;
    }
    derivedRotate_ = rotate_;
    derivedScale_ = scale_;
    derivedTranslate_ = translate_;
  }

  SceneNode* SceneManager::createSceneNode( SceneNode* parent )
  {
    auto node = new SceneNode( parent );
    sceneGraph_.insert( node );
    return node;
  }

  void SceneManager::addSceneNode( SceneNode* node )
  {
    sceneGraph_.insert( node );
  }

  void SceneManager::destroySceneNode( SceneNode* node )
  {
    sceneGraph_.erase( node );
    delete node;
  }

}