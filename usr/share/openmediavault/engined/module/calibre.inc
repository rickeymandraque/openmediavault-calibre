<?php
/**
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (C) 2009-2014 Volker Theile
 * @copyright Copyright (C) 2014-2016 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
require_once 'openmediavault/config.inc';
require_once 'openmediavault/error.inc';
require_once 'openmediavault/module.inc';
require_once 'openmediavault/systemctl.inc';

class OMVModuleCalibre extends OMVModuleServiceAbstract implements
    OMVINotifyListener,
    OMVIModuleServiceStatus
{
    public function getName()
    {
        return "Calibre";
    }

    private function getPluginName()
    {
        return strtolower( $this->getName() );
    }

    private function getXPath()
    {
        return sprintf( "/config/services/%s", $this->getPluginName() );
    }

    private function getEventMessagePath()
    {
        return sprintf( "org.openmediavault.services.%s", $this->getPluginName() );
    }

    private function getSharedFolderEventMessagePath()
    {
        return "org.openmediavault.system.shares.sharedfolder";
    }

    public function getStatus()
    {
        global $xmlConfig;

        $object = $xmlConfig->get( $this->getXPath() );
        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPath()
            );
        }
        $systemCtl = new OMVSystemCtl( $this->getPluginName() );
        return [
            'name'    => $this->getName(),
            'title'   => gettext('Calibre'),
            'enabled' => boolval($object['enable']),
            'running' => $systemCtl->isActive(),
        ];
    }

    public function applyConfig()
    {
        $cmd = sprintf("export LANG=C; omv-mkconf %s 2>&1",
            $this->getPluginName()
        );

        if (0 !== $this->exec($cmd, $output)) {
            throw new OMVException(
                OMVErrorMsg::E_EXEC_FAILED,
                $cmd,
                implode(PHP_EOL, $output)
            );
        }
    }

    public function startService()
    {
        global $xmlConfig;

        $object = $xmlConfig->get( $this->getXPath() );
        if (is_null($object)) {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPath()
            );
        }
        if (!$object['enable']) {
            return;
        }
        $systemCtl = new OMVSystemCtl($this->getPluginName());
        $systemCtl->enable(true);
    }

    public function stopService()
    {
        $systemCtl = new OMVSystemCtl($this->getPluginName());
        $systemCtl->disable(true);
    }

    public function bindListeners(OMVNotifyDispatcher $dispatcher)
    {
        $dispatcher->addListener(
            OMV_NOTIFY_MODIFY,
            $this->getEventMessagePath(),
            array($this, "setDirty")
        );

        $dispatcher->addListener(
            OMV_NOTIFY_MODIFY,
            $this->getSharedFolderEventMessagePath(),
            array($this, "setDirty")
        );
    }
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVModuleCalibre());
