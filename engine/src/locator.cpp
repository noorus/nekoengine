#include "pch.h"
#include "locator.h"

namespace neko {

  MemoryPtr Locator::memoryService_;
  ConsolePtr Locator::consoleService_;
  MessagingPtr Locator::messagingService_;
  DirectorPtr Locator::directorService_;
  MeshGeneratorPtr Locator::meshGeneratorService_;
  FileSystemPtr Locator::fileSystemService_;

}