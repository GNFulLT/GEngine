#ifndef GAME_H
#define GAME_H

#include "engine/gapplication_impl.h"
#include <filesystem>

class GGameInpl : public GApplicationImpl
{
public:
	GGameInpl(const std::filesystem::path& gprojectPath);
	virtual bool before_update() override;
	virtual void update() override;
	virtual void after_update() override;
	virtual bool before_render() override;
	virtual void render() override;
	virtual void after_render() override;
	virtual bool init(GEngine* engine) override;
	virtual void destroy() override;
private:

	std::filesystem::path m_gameProjectPath;
};

#endif // GAME_H