-- DB update 2020_11_25_01 -> 2020_11_27_00
DROP PROCEDURE IF EXISTS `updateDb`;
DELIMITER //
CREATE PROCEDURE updateDb ()
proc:BEGIN DECLARE OK VARCHAR(100) DEFAULT 'FALSE';
SELECT COUNT(*) INTO @COLEXISTS
FROM information_schema.COLUMNS
WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'version_db_world' AND COLUMN_NAME = '2020_11_25_01';
IF @COLEXISTS = 0 THEN LEAVE proc; END IF;
START TRANSACTION;
ALTER TABLE version_db_world CHANGE COLUMN 2020_11_25_01 2020_11_27_00 bit;
SELECT sql_rev INTO OK FROM version_db_world WHERE sql_rev = '1605661689213920900'; IF OK <> 'FALSE' THEN LEAVE proc; END IF;
--
-- START UPDATING QUERIES
--

INSERT INTO `version_db_world` (`sql_rev`) VALUES ('1605661689213920900');

/* Copy FemaleText into missing MaleText for Computer NPC in Mimiron fight, Ulduar. */

UPDATE `broadcast_text` SET `MaleText`=`FemaleText` WHERE `ID` IN (34284,34283,34282,34281,34280,34279,34278,34277,34276,34275,34274,34273,34268);
UPDATE `broadcast_text_locale` SET `MaleText`=`FemaleText` WHERE `ID` IN (34284,34283,34282,34281,34280,34279,34278,34277,34276,34275,34274,34273,34268);

--
-- END UPDATING QUERIES
--
COMMIT;
END //
DELIMITER ;
CALL updateDb();
DROP PROCEDURE IF EXISTS `updateDb`;
