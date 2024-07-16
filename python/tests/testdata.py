
import json
import os
from pkgutil import get_data
from typing import Any, Tuple, Type, TypeVar, Generic, List, Optional
from pydantic import BaseModel, Field, ValidationError

T = TypeVar('T', bound=BaseModel)

class InvalidLocalIds(BaseModel):
    local_id_str: str = Field(..., alias="input")
    expected_error_messages: List[str] = Field(..., alias="expectedErrorMessages")

class LocalIdTestData(BaseModel):
    invalid_local_ids: List[InvalidLocalIds] = Field(..., alias="InvalidLocalIds")

class GmodPathTestItem(BaseModel):
    path: str = Field(..., alias="path")
    vis_version: str = Field(..., alias="visVersion")

class GmodPathTestData(BaseModel):
    valid: List[GmodPathTestItem] = Field(..., alias="Valid")
    invalid: List[GmodPathTestItem] = Field(..., alias="Invalid")

class CodebookTestData(BaseModel):
    valid_position: List[List[str]] = Field(..., alias="ValidPosition")
    positions: List[List[str]] = Field(..., alias="Positions")
    states: List[List[str]] = Field(..., alias="States")
    tag: List[List[str]] = Field(..., alias="Tag")
    detail_tag: List[List[str]] = Field(..., alias="DetailTag")

class LocationsTestDataItem(BaseModel):
    value: str = Field(..., alias="value")
    success: bool = Field(..., alias="success")
    output: Optional[str] = Field(..., alias="output")
    expected_error_messages: List[str] = Field(..., alias="expectedErrorMessages")

class LocationsTestData(BaseModel):
    locations: List[LocationsTestDataItem] = Field(..., alias="locations")

class IndividualizableSetData(BaseModel):
    is_full_path: bool = Field(..., alias="isFullPath")
    vis_version: str = Field(..., alias="visVersion")
    path: str = Field(..., alias="path")
    expected: Optional[List[List[str]]] = Field(..., alias="expected")

class IndividualizableSetDataList(BaseModel):
    data: List[IndividualizableSetData] = Field(..., alias="data")
    


class TestData:

    @staticmethod
    def get_data(test_name: str, cls: Type[T]) -> T:
        cls_list = [cls]
        path = os.path.join("testdata", f"{test_name}.json")
        with open(path, 'r') as file:
            test_data_json = file.read()
            test_data_json = test_data_json.replace("\n", "")
            if(test_name == "IndividualizableSets"): 
                test_data_json = '{"data":' + test_data_json + "}"
        try:
            data_dict = json.loads(test_data_json)
            return cls.model_validate(data_dict)
        
        
        except ValidationError as e:
        
            raise Exception(f"Couldn't deserialize: {cls.__name__}") from e
        
    @staticmethod
    def get_gmodpath_data(test_name: str) -> GmodPathTestData:
        return TestData.get_data(test_name, GmodPathTestData)

    @staticmethod
    def get_local_id_data(test_name: str) -> LocalIdTestData:
        return TestData.get_data(test_name, LocalIdTestData)

    @staticmethod
    def get_codebook_data(test_name: str) -> CodebookTestData:
        return TestData.get_data(test_name, CodebookTestData)

    @staticmethod
    def get_locations_data(test_name: str) -> LocationsTestData:
        return TestData.get_data(test_name, LocationsTestData)
    
    @staticmethod
    def get_Individualizable_sets_data(test_name: str) -> IndividualizableSetDataList:
        return TestData.get_data(test_name, IndividualizableSetDataList)




